const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const multer = require("multer");
const upload = multer();
const app = express();
const port = 3000;
const mqtt = require("mqtt");
const axios = require("axios");
const { spawn } = require("child_process");

const api = require("./configs/api.config");
const db = require("./configs/db.config");

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(upload.array());
app.use(express.static("public"));

const client = mqtt.connect("ws://broker.emqx.io:8083/mqtt");
const electricalDataTopic = "2024/2005021/esp32/telemetry/";

client.on("connect", () => {
  client.subscribe(`${electricalDataTopic}#`);
});

client.on("message", (topic, message) => {
  handleMqttMessage(topic, message);
});

const appRouteElectricity = require("./routes/route-electricity");
const appRouteElectricityAnomaly = require("./routes/route-electricity-anomaly");

app.use("/api/electricity/", appRouteElectricity);
app.use("/api/electricity/anomaly/", appRouteElectricityAnomaly);

// listen on the port
app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});

app.use(function (req, res, next) {
  res.status(404);
  // respond with json
  if (req.accepts("json")) {
    res.json({ error: "Not found" });
    return;
  }

  // default to plain-text. send()
  res.type("txt").send("Not found");
});

const extractDeviceId = (topic) => {
  return topic.split("/")[4]; // Extract device ID from topic
};

const handleMqttMessage = (topic, message) => {
  if (topic.startsWith(electricalDataTopic)) {
    handleElectricalDataMessage(topic, message);
  }
};

const handleElectricalDataMessage = async (topic, message) => {
  try {
    const deviceId = extractDeviceId(topic);
    const msg = JSON.parse(message.toString());

    // Update electricalData with received data
    const electricalData = {
      voltage: msg.voltage,
      current: msg.current,
      power: msg.power,
      energy: msg.energy,
      frequency: msg.frequency,
      power_factor: msg.power_factor,
      device_id: deviceId,
    };

    // Store electrical data
    const response = await axios.post(
      api.API_STORE_ELECTRICITY_DATA,
      electricalData
    );

    const data_id = response.data.id;

    // call the anomaly detection script
    if (electricalData.power > 0.5) {
      energyIDS_script(electricalData, deviceId, data_id);
    } else {
      console.log("Power is zero. Skipping anomaly detection");
    }
  } catch (error) {
    console.error(error);
  }
};

const energyIDS_script = (electricalData, deviceId, data_id) => {
  try {
    // Call the Python script for anomaly detection
    const pythonProcess = spawn("python", [
      "../ids_models/Energy_IDS_Predict.py",
    ]);
    pythonProcess.stdin.write(
      JSON.stringify({ data: electricalData, device_id: deviceId })
    );
    pythonProcess.stdin.end();

    pythonProcess.stdout.on("data", (data) => {
      const output = JSON.parse(data.toString());
      console.log(`Python output: ${data}`);
      if_labels = output[0].if_labels == "anomaly" ? 1 : 0;
      rf_labels = output[0].rf_labels == "anomaly" ? 1 : 0;
      if (if_labels == 1 || rf_labels == 1) {
        const data = {
          data_id: data_id,
          if_labels_anomaly: if_labels,
          rf_labels_anomaly: rf_labels,
        };

        // Store anomaly data
        const response = axios.post(api.API_STORE_ANOMALY_DATA, data);
      }
    });

    pythonProcess.stderr.on("data", (data) => {
      console.error(`Python error: ${data}`);
    });
  } catch (error) {
    console.error(error);
  }
};
