const {electricity, validate} = require('../controllers/electricity-controller');

const router = require('express').Router();

router.get('/retrieve', electricity.getElectricityData);
router.get('/get/:id', electricity.getElectricityDataByID);
router.get('/get/monthly/:month', electricity.getElectricityDataByMonth);
router.post('/add', validate("addElectricityData"), electricity.addElectricityData);
router.get('/total/energy/get', electricity.getTotalEnergyConsumption);
router.get('/total/power/get', electricity.getTotalPowerConsumption);
router.get('/total/current/get', electricity.getTotalCurrentConsumption);
router.get('/total/current/month/:month/get', electricity.getTotalCurrentConsumptionPerMonth);
module.exports = router;