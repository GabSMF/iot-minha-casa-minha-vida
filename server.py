import json
import ssl
import certifi
import paho.mqtt.client as mqtt
from os import environ as env
from urllib.parse import quote_plus, urlencode
from dotenv import find_dotenv, load_dotenv
from authlib.integrations.flask_client import OAuth
from flask import Flask, redirect, render_template, session, url_for

# Variáveis de Ambiente
ENV_FILE = find_dotenv()
if ENV_FILE:
    load_dotenv(ENV_FILE)

# Callbacks
def on_connect(client, userdata, flags, reason_code, properties=None):
    if reason_code == mqtt.CONNACK_ACCEPTED:
        print(f"MQTT connected successfully (code {reason_code})")
        client.subscribe("projcasa/users/+/login")
    else:
        print(f"MQTT connection refused (code {reason_code})")

def on_message(client, userdata, msg):
    print(f"Received on {msg.topic}: {msg.payload.decode()}")

def on_publish(client, userdata, mid, reason_code=None, properties=None):
    print(f"Message {mid} published")

# Username e Password MQTT
def set_mqtt_credentials(mqttc):
    user = env.get("MQTT_USER")
    pwd  = env.get("MQTT_PASSWORD")
    if user and pwd:
        mqttc.username_pw_set(user, pwd)
    else:
        print("No MQTT credentials provided; attempting anonymous connection")

# ----- Setup MQTT -----

# MQTT Client 
mqtt_client = mqtt.Client(
    client_id="projcasa_flask",
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2
)

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.on_publish = on_publish

# Host e Port
mqtt_broker = env.get("MQTT_BROKER_HOST", "localhost")
mqtt_port   = int(env.get("MQTT_BROKER_PORT", 8883))

set_mqtt_credentials(mqtt_client)

# Certificados
mqtt_client.tls_set(
    ca_certs=certifi.where(),
    tls_version=ssl.PROTOCOL_TLS_CLIENT
)
mqtt_client.tls_insecure_set(False)

# Connect loop
try:
    mqtt_client.connect(mqtt_broker, mqtt_port, keepalive=60)
    mqtt_client.loop_start()
except Exception as e:
    print(f"[WARNING] Unable to connect to MQTT broker at {mqtt_broker}:{mqtt_port} → {e}")

# ----- Auth0 e Flask -----
app = Flask(__name__)
app.secret_key = env.get("APP_SECRET_KEY")

oauth = OAuth(app)

# Setup Auth0
oauth.register(
    "auth0",
    client_id=env.get("AUTH0_CLIENT_ID"),
    client_secret=env.get("AUTH0_CLIENT_SECRET"),
    client_kwargs={"scope": "openid profile email"},
    server_metadata_url=(
        f'https://{env.get("AUTH0_DOMAIN")}/.well-known/openid-configuration'
    )
)

# ----- Rotas Flask -----
# Redireciona pra página de login do Auth0
@app.route("/login")
def login():
    redirect_uri = url_for("callback", _external=True)
    return oauth.auth0.authorize_redirect(redirect_uri=redirect_uri)

# Retorna dá pagina de login do auth0 pra página inicial logada e faz post dos dados de usuário pro MQTT
@app.route("/callback", methods=["GET", "POST"])
def callback():
    token = oauth.auth0.authorize_access_token()
    session["user"] = token

    # User id
    user_info = token.get("userinfo") or oauth.auth0.parse_id_token(token)
    user_id = user_info["sub"]

    # Publish pro MQTT
    topic = f"projcasa/users/{user_id}/login"
    payload = json.dumps({
        "user_id": user_id,
        "timestamp": token.get("expires_at")
    })
    if mqtt_client.is_connected():
        mqtt_client.publish(topic, payload)
    else:
        print(f"Não foi possível conectar: MQTT não conectado. Topic {topic}")

    return redirect("/")

# Logout, esvazia a session e redireciona pra página inicial

@app.route("/logout")
def logout():
    session.clear()
    return redirect(
        "https://" + env.get("AUTH0_DOMAIN")
        + "/v2/logout?"
        + urlencode({
            "returnTo": url_for("home", _external=True),
            "client_id": env.get("AUTH0_CLIENT_ID"),
        }, quote_via=quote_plus)
    )

# Home com botão para Login
@app.route("/")
def home():
    return render_template(
        "home.html",
        session=session.get("user"),
        pretty=json.dumps(session.get("user"), indent=4)
    )

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(env.get("PORT", 3000)))
