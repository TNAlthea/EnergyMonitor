const { spawn } = require('child_process');
const axios = require('axios');

const handleMqttMessage = async (topic, message) => {
  try {
    const deviceId = extractDeviceId(topic);
    const msg = JSON.parse(message.toString());

    const electricalData = {
      voltage: msg.voltage,
      current: msg.current,
      power: msg.power,
      energy: msg.energy,
      frequency: msg.frequency,
      power_factor: msg.power_factor,
    };

    // Save electrical data
    const response = await axios.post(api.API_STORE_ELECTRICITY_DATA, electricalData);

    // Call the Python script for anomaly detection
    const pythonProcess = spawn('python', ['predict.py']);
    pythonProcess.stdin.write(JSON.stringify({ data: electricalData, device_id: deviceId }));
    pythonProcess.stdin.end();

    pythonProcess.stdout.on('data', (data) => {
      console.log(`Python output: ${data}`);
    });

    pythonProcess.stderr.on('data', (data) => {
      console.error(`Python error: ${data}`);
    });
  } catch (error) {
    console.error(error);
  }
};
