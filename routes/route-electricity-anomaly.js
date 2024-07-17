const {electricity_anomaly, validate} = require('../controllers/electricity-anomaly-controller');

const router = require('express').Router();

router.post('/add', validate("addElectricityAnomalyData"), electricity_anomaly.addElectricityAnomalyData);

module.exports = router;