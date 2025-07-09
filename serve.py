import eventlet
eventlet.monkey_patch()

from flask_socketio import SocketIO

from flask import Flask, render_template, request, redirect, url_for, session
from flask_sqlalchemy import SQLAlchemy
from authlib.integrations.flask_client import OAuth
from os import environ as env
from urllib.parse import quote_plus, urlencode
from datetime import datetime
import json
from functools import wraps
from dotenv import find_dotenv, load_dotenv
import paho.mqtt.client as mqtt
import ssl
import certifi

app = Flask(__name__)
socketio = SocketIO(app)

# Variáveis de Ambiente
ENV_FILE = find_dotenv()
if ENV_FILE:
    load_dotenv(ENV_FILE)

# Conectar MQTT usando variáveis do .env
mqtt_broker = env.get("MQTT_BROKER_HOST")
mqtt_port = int(env.get("MQTT_BROKER_PORT"))
mqtt_user = env.get("MQTT_USER")
mqtt_password = env.get("MQTT_PASSWORD")

mqtt_client = mqtt.Client(client_id="flask_server")
mqtt_client.username_pw_set(mqtt_user, mqtt_password)
mqtt_client.tls_set(
    ca_certs=certifi.where(),
    tls_version=ssl.PROTOCOL_TLS_CLIENT
)
mqtt_client.tls_insecure_set(False)

try:
    mqtt_client.connect(mqtt_broker, mqtt_port, keepalive=60)
    mqtt_client.loop_start()
except Exception as e:
    print(f"[WARNING] MQTT connection failed: {e}")


def on_mqtt_message(client, userdata, msg):
    print(f"MQTT update recebido: {msg.topic} | payload: {msg.payload}")
    socketio.emit('preferences_updated', {'update': True})

mqtt_client.on_message = on_mqtt_message
mqtt_client.subscribe("/updates")

# Configuração do banco de dados PostgreSQL
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://iot:pepcon-garton@postgresql.janks.dev.br:5432/projeto'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.secret_key = env.get("APP_SECRET_KEY", "supersecretkey")

db = SQLAlchemy(app)

# Configuração do Auth0
oauth = OAuth(app)
oauth.register(
    "auth0",
    client_id=env.get("AUTH0_CLIENT_ID"),
    client_secret=env.get("AUTH0_CLIENT_SECRET"),
    client_kwargs={"scope": "openid profile email"},
    server_metadata_url=f'https://{env.get("AUTH0_DOMAIN")}/.well-known/openid-configuration'
)

class Usuario(db.Model):
    __tablename__ = 'grupo2_usuario'
    id_usuario = db.Column(db.String, primary_key=True)
    email = db.Column(db.String, nullable=False)
    nome = db.Column(db.String, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

class Dispositivo(db.Model):
    __tablename__ = 'grupo2_dispositivo'
    id_dispositivo = db.Column(db.String, primary_key=True)
    fk_id_usuario = db.Column(db.String, db.ForeignKey('grupo2_usuario.id_usuario'))
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    type = db.Column(db.String, nullable=False)
    nome = db.Column(db.String, nullable=True)  # Novo campo

class Preferences(db.Model):
    __tablename__ = 'grupo2_preferences'
    created_at = db.Column(db.DateTime, primary_key=True, default=datetime.utcnow)
    id_dispositivo = db.Column(db.String, db.ForeignKey('grupo2_dispositivo.id_dispositivo'))
    ar_temperatura = db.Column(db.Integer, nullable=True)
    ar_direcao = db.Column(db.String(50), nullable=True)
    ar_modo = db.Column(db.String(50), nullable=True)
    ar_velocidade = db.Column(db.String(50), nullable=True)
    luz_acesa = db.Column(db.Boolean, nullable=True)
    luz_cor = db.Column(db.String(50), nullable=True)
    luz_intensidade = db.Column(db.Integer, nullable=True)
    luz_modo = db.Column(db.String(50), nullable=True)
    sensor_oversampling = db.Column(db.String(50), nullable=True)
    sensor_iir = db.Column(db.String(50), nullable=True)

def notify_preferences_update(id_dispositivo):
    print("ENTROU AQUI!")
    if mqtt_client.is_connected():
        print("eoeoeo")
        mqtt_client.publish("/updates", id_dispositivo)
    else:
        print("MQTT não conectado, não foi possível notificar update.")

@app.context_processor
def inject_usuario():
    user_id = session.get("id_usuario")
    usuario = Usuario.query.filter_by(id_usuario=user_id).first() if user_id else None

    # Flags para navegação dinâmica
    dispositivo_arcondicionado = False
    dispositivo_lampada = False
    dispositivo_sensores = False

    if user_id:
        dispositivos = Dispositivo.query.filter_by(fk_id_usuario=user_id).all()
        for d in dispositivos:
            if d.type == 'arcondicionado':
                dispositivo_arcondicionado = True
            elif d.type == 'luz':
                dispositivo_lampada = True
            elif d.type == 'sensor':
                dispositivo_sensores = True

    return dict(
        usuario=usuario,
        dispositivo_arcondicionado=dispositivo_arcondicionado,
        dispositivo_lampada=dispositivo_lampada,
        dispositivo_sensores=dispositivo_sensores
    )

def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if "id_usuario" not in session:
            return redirect(url_for("login"))
        return f(*args, **kwargs)
    return decorated_function

# Rotas de autenticação
@app.route("/login")
def login():
    redirect_uri = url_for("callback", _external=True)
    return oauth.auth0.authorize_redirect(redirect_uri=redirect_uri)

@app.route("/callback")
def callback():
    token = oauth.auth0.authorize_access_token()
    session["user"] = token

    user_info = token.get("userinfo") or oauth.auth0.parse_id_token(token)
    session["id_usuario"] = user_info["sub"]

    # Registro do usuário
    usuario = Usuario.query.filter_by(id_usuario=user_info["sub"]).first()
    if not usuario:
        usuario = Usuario(
            id_usuario=user_info["sub"],
            email=user_info.get("email", ""),
            nome=user_info.get("name", "")
        )
        db.session.add(usuario)
        db.session.commit()

    return redirect("/home")

@app.route("/logout")
def logout():
    session.clear()
    return redirect(
        "https://" + env.get("AUTH0_DOMAIN")
        + "/v2/logout?"
        + urlencode({
            "returnTo": url_for("homeLogin", _external=True),
            "client_id": env.get("AUTH0_CLIENT_ID"),
        }, quote_via=quote_plus)
    )

@app.route("/")
def homeLogin():
    return render_template("homeLogin.html")

@app.route("/home")
@login_required
def home():
    user_id = session["id_usuario"]
    dispositivos = Dispositivo.query.filter_by(fk_id_usuario=user_id).all()
    return render_template("home.html", dispositivos=dispositivos)

@app.route("/arcondicionado", methods=["GET", "POST"])
@login_required
def arcondicionado():
    user_id = session["id_usuario"]
    dispositivo = Dispositivo.query.filter_by(fk_id_usuario=user_id, type='arcondicionado').first()
    if not dispositivo:
        return "Nenhum ar-condicionado cadastrado para este usuário.", 404

    preferences = Preferences.query.filter_by(id_dispositivo=dispositivo.id_dispositivo).first()

    if request.method == "POST":
        if preferences:
            preferences.ar_temperatura = request.form.get("intensity")
            preferences.ar_direcao = request.form.get("direcao")
            preferences.ar_modo = request.form.get("modoAr")
            preferences.ar_velocidade = request.form.get("velocidade")
        else:
            preferences = Preferences(
                id_dispositivo=dispositivo.id_dispositivo,
                ar_temperatura=request.form.get("intensity"),
                ar_direcao=request.form.get("direcao"),
                ar_modo=request.form.get("modoAr"),
                ar_velocidade=request.form.get("velocidade"),
            )
            db.session.add(preferences)
        db.session.commit()
        notify_preferences_update(dispositivo.id_dispositivo)
        return redirect(url_for("arcondicionado"))

    return render_template("arcondicionado.html", preferences=preferences)

@app.route("/luz", methods=["GET", "POST"])
@login_required
def luz():
    user_id = session["id_usuario"]
    dispositivo = Dispositivo.query.filter_by(fk_id_usuario=user_id, type='luz').first()
    if not dispositivo:
        return "Nenhuma lâmpada cadastrada para este usuário.", 404

    preferences = Preferences.query.filter_by(id_dispositivo=dispositivo.id_dispositivo).first()

    if request.method == "POST":
        if preferences:
            preferences.luz_acesa = request.form.get("switch") == "on"
            preferences.luz_cor = request.form.get("color")
            preferences.luz_intensidade = request.form.get("intensity")
            preferences.luz_modo = request.form.get("modo")
        else:
            preferences = Preferences(
                id_dispositivo=dispositivo.id_dispositivo,
                luz_acesa=request.form.get("switch") == "on",
                luz_cor=request.form.get("color"),
                luz_intensidade=request.form.get("intensity"),
                luz_modo=request.form.get("modo"),
            )
            db.session.add(preferences)
        db.session.commit()
        print("TESTE")
        notify_preferences_update(dispositivo.id_dispositivo)
        return redirect(url_for("luz"))

    return render_template("luz.html", preferences=preferences)

@app.route("/sensores", methods=["GET", "POST"])
@login_required
def sensores():
    user_id = session["id_usuario"]
    dispositivo = Dispositivo.query.filter_by(fk_id_usuario=user_id, type='sensor').first()
    if not dispositivo:
        return "Nenhum sensor cadastrado para este usuário.", 404

    preferences = Preferences.query.filter_by(id_dispositivo=dispositivo.id_dispositivo).first()

    if request.method == "POST":
        if preferences:
            preferences.sensor_oversampling = request.form.get("oversampling")
            preferences.sensor_iir = request.form.get("iir-filter")
        else:
            preferences = Preferences(
                id_dispositivo=dispositivo.id_dispositivo,
                sensor_oversampling=request.form.get("oversampling"),
                sensor_iir=request.form.get("iir-filter"),
            )
            db.session.add(preferences)
        db.session.commit()
        notify_preferences_update(dispositivo.id_dispositivo)
        return redirect(url_for("sensores"))

    return render_template("sensores.html", preferences=preferences)

@app.route("/dispositivos")
@login_required
def dispositivos():
    user_id = session["id_usuario"]
    dispositivos = Dispositivo.query.filter_by(fk_id_usuario=user_id).all()
    error = request.args.get("error")
    return render_template("dispositivos.html", dispositivos=dispositivos, error=error)

@app.route("/add_device", methods=["POST"])
@login_required
def add_device():
    user_id = session["id_usuario"]
    id_dispositivo = request.form.get("id_dispositivo")
    nome = request.form.get("nome")
    if not id_dispositivo:
        return redirect(url_for("dispositivos"))
    dispositivo = Dispositivo.query.filter_by(id_dispositivo=id_dispositivo).first()
    if not dispositivo:
        from urllib.parse import urlencode
        params = urlencode({"error": "Dispositivo não encontrado."})
        return redirect(f"{url_for('dispositivos')}?{params}")
    dispositivo.fk_id_usuario = user_id
    if nome:
        dispositivo.nome = nome
    db.session.commit()
    return redirect(url_for("dispositivos"))

@app.route("/delete_device/<id_dispositivo>", methods=["POST"])
@login_required
def delete_device(id_dispositivo):
    user_id = session["id_usuario"]
    dispositivo = Dispositivo.query.filter_by(id_dispositivo=id_dispositivo, fk_id_usuario=user_id).first()
    if dispositivo:
        dispositivo.fk_id_usuario = None
        dispositivo.nome = None  
        db.session.commit()
    return redirect(url_for("dispositivos"))

@app.route("/edit_device_name/<id_dispositivo>", methods=["POST"])
@login_required
def edit_device_name(id_dispositivo):
    user_id = session["id_usuario"]
    nome = request.form.get("nome")
    dispositivo = Dispositivo.query.filter_by(id_dispositivo=id_dispositivo, fk_id_usuario=user_id).first()
    if dispositivo:
        dispositivo.nome = nome
        db.session.commit()
    return redirect(url_for("dispositivos"))

if __name__ == "__main__":
    with app.app_context():
        db.create_all()
    socketio.run(app, host="0.0.0.0", port=int(env.get("PORT", 3001)))