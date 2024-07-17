const config = require("../configs/db.config");
const mysql = require("mysql");
const pool = mysql.createPool(config);
const db = require("../configs/db.config");
const { body, validationResult } = require("express-validator");

pool.on("error", (err) => {
  console.error(err);
});

async function checkDataId(req, res) {
  let connection;
  try {
    // Wait for the connection to be established
    connection = await new Promise((resolve, reject) => {
      pool.getConnection((err, conn) => {
        if (err) reject(err);
        else resolve(conn);
      });
    });

    // Wait for the query results
    const result = await new Promise((resolve, reject) => {
      connection.query(
        `SELECT data_id FROM electricity_monitor WHERE data_id = ?`,
        [req.body.data_id],
        (error, results) => {
          if (error) reject(error);
          else resolve(results);
        }
      );
    });

    // Properly release the connection
    connection.release();

    // Return true if at least one record exists
    return result.length > 0;
  } catch (error) {
    console.error(error);
    if (connection) connection.release(); // Ensure connection is released in case of error
    res.status(500).send({
      code: 500,
      success: false,
      message: "Failed to check data ID!",
      error: error,
    });
    return false; // Consider how you want to handle this case in your application logic
  }
}

const validate = (method) => {
  switch (method) {
    case "addElectricityAnomalyData": {
      return [
        body("data_id")
          .notEmpty()
          .withMessage("data_id is required")
          .isNumeric()
          .withMessage("data_id must be a string")
          .escape(),
        body("if_labels_anomaly")
          .notEmpty()
          .withMessage("if_labels_anomaly is required")
          .isBoolean()
          .escape(),
        body("rf_labels_anomaly")
          .notEmpty()
          .withMessage("if_labels_anomaly is required")
          .isBoolean()
          .escape(),
      ];
    }
  }
};

const addElectricityAnomalyData = async (req, res) => {
  const errors = validationResult(req);

  if (!errors.isEmpty()) {
    return res.status(422).send({
      code: 422,
      success: false,
      message: "Missing required parameter!",
      errors: errors.array(),
    });
  }

  try {
    const dataExists = await checkDataId(req, res);
    if (!dataExists) {
      return res.status(404).send({
        code: 404,
        success: false,
        message: "Data not found!",
      });
    }

    pool.getConnection(function (err, connection) {
      if (err) {
        return res.status(500).send({
          code: 500,
          success: false,
          message: "Failed to get database connection!",
          error: err,
        });
      }
      connection.query(
        `INSERT INTO electricity_anomaly SET ?`,
        req.body,
        function (error, results) {
          if (error) {
            // Instead of throwing the error, send a response to the client
            return res.status(500).send({
              code: 500,
              success: false,
              message: "Failed to insert data into database",
              error: error,
            });
          }
          // Send a success response only if there was no error
          res.status(200).send({
            code: 200,
            success: true,
            message: "Data inserted successfully",
            data: req.body,
          });
          // Make sure to release the connection after use
          connection.release();
        }
      );
    });
  } catch (error) {}
};

module.exports = {
  validate,
  electricity_anomaly: {
    addElectricityAnomalyData,
  },
};
