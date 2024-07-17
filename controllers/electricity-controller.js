const config = require("../configs/db.config");
const mysql = require("mysql");
const pool = mysql.createPool(config);
const { body, validationResult } = require("express-validator");

pool.on("error", (err) => {
  console.error(err);
});

const validate = (method) => {
  switch (method) {
    case "addElectricityData": {
      return [
        body("voltage")
          .notEmpty()
          .withMessage("Voltage is required")
          .isFloat()
          .withMessage("Voltage must be a float")
          .escape(),
        body("current")
          .notEmpty()
          .withMessage("Current is required")
          .isFloat()
          .withMessage("Current must be a float")
          .escape(),
        body("power")
          .notEmpty()
          .withMessage("Power is required")
          .isFloat()
          .withMessage("Power must be a float")
          .escape(),
        body("energy")
          .notEmpty()
          .withMessage("Energy is required")
          .isFloat()
          .withMessage("Energy must be a float")
          .escape(),
        body("frequency")
          .notEmpty()
          .withMessage("Frequency is required")
          .isFloat()
          .withMessage("Frequency must be a float")
          .escape(),
        body("power_factor")
          .notEmpty()
          .withMessage("Power factor is required")
          .isFloat()
          .withMessage("Power factor must be a float")
          .escape(),
      ];
    }
  }
};

const getElectricityData = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT * FROM electricity_monitor;
      `,
      function (error, results) {
        if (error) throw error;
        res.status(200).send({
          code: 200,
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const getElectricityDataByID = (req, res) => {
  let id = req.params.id;
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT * FROM electricity_monitor WHERE id = ?;
      `,
      [id],
      function (error, results) {
        if (error) throw error;
        res.send({
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const getElectricityDataByMonth = (req, res) => {
  let month = req.params.month;
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT * FROM electricity_monitor WHERE MONTH(created_at) = ?;
      `,
      [month],
      function (error, results) {
        if (error) throw error;
        res.send({
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const addElectricityData = (req, res) => {
  const errors = validationResult(req);

  if (!errors.isEmpty()) {
    res.status(422).send({
      code: 422,
      success: false,
      message: "Missing required parameter!",
      errors: errors.array(),
    });
    return;
  }

  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `INSERT INTO electricity_monitor SET ?`,
      req.body,
      function (error, result) {
        if (error) throw error;
        res.status(200).send({
          code: 200,
          success: true,
          message: "Berhasil tambah ambil data!",
          data: req.body,
          id: result.insertId,
        });
      }
    );
    connection.release();
  });
};

const getTotalEnergyConsumption = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT SUM(energy) as total_energy FROM electricity_monitor;
      `,
      function (error, results) {
        if (error) throw error;
        res.status(200).send({
          code: 200,
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const getTotalPowerConsumption = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT SUM(power) as total_power FROM electricity_monitor;
      `,
      function (error, results) {
        if (error) throw error;
        res.status(200).send({
          code: 200,
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const getTotalCurrentConsumption = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
    SELECT SUM(current) as total_current FROM electricity_monitor;
    `,
      function (error, results) {
        if (error) throw error;
        res.status(200).send({
          code: 200,
          success: true,
          message: "Berhasil ambil data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

const getTotalCurrentConsumptionPerMonth = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }

    if (
      !req.params ||
      req.params.month === undefined ||
      req.params.month === null ||
      req.params.month === "" ||
      req.params.month === 0
    ) {
      res.status(422).send({
        code: 422,
        success: false,
        message: "Missing required parameter!",
        data: { body: req.body },
      });

      return;
    }

    const query = `
    SELECT SUM(current) as total_current, MONTH(created_at) as month 
    FROM electricity_monitor 
    WHERE MONTH(created_at) = ?
  `;

    connection.query(query, [req.params.month], function (error, results) {
      if (error) {
        res.status(500).send({
          code: 500,
          success: false,
          message: "Query execution failed!",
          error: error,
        });
        return;
      }
      res.status(200).send({
        code: 200,
        success: true,
        message: "Berhasil ambil data!",
        data: results,
      });
    });
    connection.release();
  });
};

const getTotalPowerConsumptionPerMonth = (req, res) => {
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }

    if (
      !req.params ||
      req.params.month === undefined ||
      req.params.month === null ||
      req.params.month === "" ||
      req.params.month === 0
    ) {
      res.status(422).send({
        code: 422,
        success: false,
        message: "Missing required parameter!",
        data: { body: req.body },
      });

      return;
    }

    const query = `
    SELECT SUM(power) as total_power, MONTH(created_at) as month 
    FROM electricity_monitor 
    WHERE MONTH(created_at) = ?
  `;

    connection.query(query, [req.params.month], function (error, results) {
      if (error) {
        res.status(500).send({
          code: 500,
          success: false,
          message: "Query execution failed!",
          error: error,
        });
        return;
      }
      res.status(200).send({
        code: 200,
        success: true,
        message: "Berhasil ambil data!",
        data: results,
      });
    });
    connection.release();
  });
};

const getDailyPowerConsumptionPerMonth = (req, res) => {
  let month = req.params.month;
  pool.getConnection(function (err, connection) {
    if (err) {
      res.status(500).send({
        code: 500,
        success: false,
        message: "Failed to get database connection!",
        error: err,
      });
      return;
    }
    connection.query(
      `
      SELECT DATE(created_at) as date, SUM(power) as total_power
      FROM electricity_monitor
      WHERE MONTH(created_at) = ?
      GROUP BY DATE(created_at);
      `,
      [month],
      function (error, results) {
        if (error) {
          res.status(500).send({
            code: 500,
            success: false,
            message: "Failed to fetch data!",
            error: error,
          });
          return;
        }
        res.status(200).send({
          code: 200,
          success: true,
          message: "Successfully fetched data!",
          data: results,
        });
      }
    );
    connection.release();
  });
};

module.exports = {
  validate,
  electricity: {
    addElectricityData,
    getElectricityData,
    getElectricityDataByID,
    getElectricityDataByMonth,
    getTotalEnergyConsumption,
    getTotalPowerConsumption,
    getTotalPowerConsumptionPerMonth,
    getDailyPowerConsumptionPerMonth,
    getTotalCurrentConsumption,
    getTotalCurrentConsumptionPerMonth,
  },
};
