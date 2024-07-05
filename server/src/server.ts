// src/server.ts
import express from "express";
import path from "path";
import fs from "fs";

const app = express();
const PORT = 3001;

const getBuildVersion = () => {
  const buildVersionPath = path.join(
    __dirname,
    "..",
    "..",
    "src",
    "defines",
    "version.h"
  );

  const data = fs.readFileSync(buildVersionPath, "utf8");

  const regex = /#define FIRMWARE_VERSION\s+"([^"]+)"/;
  const match = data.match(regex);

  if (match?.[1]) {
    return match[1];
  } else {
    console.error("FIRMWARE_VERSION not found in file");
    return null;
  }
};

app.get("/download/firmware", (req, res) => {
  const esp32Version = req.headers["x-esp32-version"];
  const buildVersion = getBuildVersion();

  console.log("new update requested", { esp32Version, buildVersion });

  if (esp32Version === buildVersion)
    return res.status(304).send("Not Modified");

  const filePath = path.join(
    __dirname,
    "..",
    "..",
    ".pio",
    "build",
    "esp32dev",
    "firmware.bin"
  );

  res.download(filePath);
});

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
