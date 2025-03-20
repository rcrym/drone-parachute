const fs = require('fs');
const csv = require('csv-parser');

const filePath = '../drop-test-data/test-jan-23-num-2-successful-deployment.csv';
const g = 9.81; // Gravity constant in m/s^2
const t_start = 573.5;
const t_end = 577.6;
const h_initial = 10.9728; // Initial height in meters

let time = [];
let accelZ = [];

// Read CSV file
fs.createReadStream(filePath)
  .pipe(csv())
  .on('data', (row) => {
    let t = parseFloat(row["Time (s)"]);
    if (t >= t_start && t <= t_end) {
      time.push(t);
      accelZ.push(parseFloat(row["Linear Acceleration z (m/s^2)"]));
    }
  })
  .on('end', () => {
    let velocity = [0]; // Initial velocity
    let height = [h_initial]; // Initial height

    // Compute velocity and height using numerical integration (trapezoidal rule)
    for (let i = 1; i < time.length; i++) {
      let dt = time[i] - time[i - 1];
      let avg_accel = (accelZ[i] + accelZ[i - 1]) / 2;
      let v_new = velocity[i - 1] + avg_accel * dt;
      let h_new = height[i - 1] + velocity[i - 1] * dt + 0.5 * avg_accel * dt ** 2;

      velocity.push(v_new);
      height.push(h_new);
    }

    // Generate chart using HTML and Chart.js
    const htmlContent = `
    <!DOCTYPE html>
    <html>
    <head>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    </head>
    <body>
      <canvas id="myChart"></canvas>
      <script>
        const ctx = document.getElementById('myChart').getContext('2d');
        const chart = new Chart(ctx, {
          type: 'line',
          data: {
            labels: ${JSON.stringify(time)},
            datasets: [
              {
                label: 'Velocity (m/s)',
                data: ${JSON.stringify(velocity)},
                borderColor: 'blue',
                fill: false,
              },
              {
                label: 'Height (m)',
                data: ${JSON.stringify(height)},
                borderColor: 'red',
                fill: false,
              },
              {
                label: 'Acceleration (m/s^2)',
                data: ${JSON.stringify(accelZ)},
                borderColor: 'green',
                fill: false,
              }
            ]
          },
          options: {
            scales: {
              x: {
                title: { display: true, text: 'Time (s)' }
              },
              y: {
                title: { display: true, text: 'Value' }
              }
            }
          }
        });
      </script>
    </body>
    </html>`;

    fs.writeFileSync('velocity_height_chart.html', htmlContent);
    console.log('Chart saved as velocity_height_chart.html');
  });
