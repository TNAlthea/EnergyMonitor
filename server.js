const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const multer = require("multer");
const upload = multer();
const app = express();
const port = 3000;

const db = require("./configs/db.config");

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(upload.array());
app.use(express.static("public"));

const appRoute = require("./routes/route-electricity");
app.use("/api/electricity/", appRoute);

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
