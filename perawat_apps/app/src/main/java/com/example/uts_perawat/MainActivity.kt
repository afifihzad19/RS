package com.example.uts_perawat

import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.example.uts_perawat.R
import org.eclipse.paho.android.service.MqttAndroidClient
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.MqttCallback
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.eclipse.paho.client.mqttv3.MqttDeliveryToken
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.json.JSONObject

class MainActivity : AppCompatActivity() {

    private lateinit var mqttClient: MqttAndroidClient
    private lateinit var temperatureTextView: TextView
    private lateinit var humidityTextView: TextView
    private lateinit var publishMessageEditText: EditText
    private lateinit var sendButton: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Inisialisasi komponen UI
        temperatureTextView = findViewById(R.id.temperatureTextView)
        humidityTextView = findViewById(R.id.humidityTextView)
        publishMessageEditText = findViewById(R.id.publishMessageEditText)
        sendButton = findViewById(R.id.sendButton)

        // Menginisialisasi klien MQTT
        initializeMQTTClient()

        // Menghubungkan ke server MQTT saat tombol "Connect" ditekan
        sendButton.setOnClickListener {
            publishMessage()
        }
    }

    private fun initializeMQTTClient() {
        val clientId = MqttClient.generateClientId()
        mqttClient = MqttAndroidClient(applicationContext, "tcp://broker.example.com:1883", clientId)

        mqttClient.setCallback( object : MqttCallback {
            override fun connectionLost(cause: Throwable?) {
                // Menangani kehilangan koneksi
            }

            override fun messageArrived(topic: String?, message: MqttMessage?) {
                // Menangani pesan yang diterima dari server MQTT
                val payload = String(message?.payload ?: byteArrayOf())
                displayMQTTData(payload)
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {
                TODO("Not yet implemented")
            }

//            override fun deliveryComplete(token: IMqttToken?) {
//                // Menangani penyelesaian pengiriman pesan
//            }
        })
    }

    private fun publishMessage() {
        try {
            val topic = "your/mqtt/topic"
            val payload = publishMessageEditText.text.toString()
            mqttClient.publish(topic, payload.toByteArray(), 0, false)
        } catch (e: MqttException) {
            e.printStackTrace()
        }
    }

    private fun displayMQTTData(data: String) {
        // Parsing data JSON dan menampilkan suhu dan kelembapan pada UI
        runOnUiThread {
            val json = JSONObject(data)
            val temperature = json.optDouble("suhu")
            val humidity = json.optDouble("kelembapan")

            temperatureTextView.text = "Temperature: $temperature"
            humidityTextView.text = "Humidity: $humidity"
        }
    }
}
