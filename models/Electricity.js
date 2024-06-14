const DataTypes = require("sequelize");

const ElectricityModel = {
  id: {
    type: DataTypes.INTEGER,
    autoIncrement: true,
    primaryKey: true,
  },
  voltage: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
  current: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
  power: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
  energy: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
  frequency: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
  power_factor: {
    type: DataTypes.FLOAT,
    allowNull: false,
    unique: true,
  },
};

module.exports = {
  initialize: (sequelize) => {
    this.model = sequelize.define("electricity", ElectricityModel);
  },

  createElectricity: (electricity) => {
    return this.model.create(electricity);
  }
};
