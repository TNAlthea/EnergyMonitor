const {electricity_anomaly, validate} = require('../controllers/electricity-anomaly-controller');

const router = require('express').Router();

router.post('/add', validate("addElectricityAnomalyData"), electricity_anomaly.addElectricityAnomalyData);
router.get('/retrieve', electricity_anomaly.getElectricityAnomalyData);
router.get('/retrieve/:device_id', electricity_anomaly.getElectricityAnomalyDataByDeviceId);

module.exports = router;