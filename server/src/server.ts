// src/server.ts
import express from "express";
import path from "path";

const app = express();
const PORT = 3001;

app.get("/download/firmware", (req, res) => {
  console.log("firmware requested");
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
