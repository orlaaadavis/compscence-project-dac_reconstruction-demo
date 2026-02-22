const express = require('express');
const path = require('path');
const fs = require('fs');
const { exec } = require('child_process');

const app = express();
const PORT = 3000;

// Serve static files (index.html in public/)
app.use(express.static(path.join(__dirname, 'public')));

// Endpoint to get CSV data
app.get('/data', (req, res) => {
    const csvPath = path.join(__dirname, '../backend/backend/output.csv');
    if (fs.existsSync(csvPath)) {
        res.sendFile(csvPath);
    } else {
        res.status(404).send('CSV file not found');
    }
});

// Endpoint to run backend with adjustable parameters
app.get('/run', (req, res) => {
    const length = req.query.length || 999;
    const amplitude = req.query.amplitude || 1;
    const bits = req.query.bits || 8;

    const exePath = path.join(__dirname, '../backend/backend/main.exe');
    const command = `"${exePath}" ${length} ${amplitude} ${bits}`;
    console.log("Running command:", command);

    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error: ${error.message}`);
            console.error(`stderr: ${stderr}`);
            return res.status(500).send('Backend error');
        }
        console.log("Backend output:", stdout);
        // Send MSE back to frontend so it knows backend finished
        res.send(stdout.trim());
    });
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
