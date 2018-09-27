/* Tyrel Hiebert
 * V00898825
 * CSC 360 - Assignment 3
 * July 15, 2018
 */

p5.disableFriendlyErrors = true;

let variations;
const DISPLAY_WAIT = true;
const DISPLAY_TURN = true;

let rawData, waitChart, turnaroundChart, waitData, turnaroundData;
let quantums = [];
let dispatches = [];
let waits = [];
let turnarounds = [];
let sortedWaits = [];
let sortedTurnarounds = [];

function preload() {
    rawData = loadStrings("output.txt");
}

function setup() {
    variations = rawData.length - 1;
    sortData();
    buildChart();
}

// pull out each attribute from the data
function sortData() {
    let data = [];
    let i,j;
    for (i in rawData) {
        let vals = rawData[i].split(' ');
        data.push(vals);
    }
    for (i = 1; i <= variations; i++) {
        if (!(quantums.includes(data[i][0]))) {quantums.push(data[i][0]);}
        if (!(dispatches.includes(data[i][1]))) {dispatches.push(data[i][1]);}
        waits.push(data[i][2]);
        turnarounds.push(data[i][3]);
    }
    for (i = 0; i < quantums.length; i++) {
        sortedWaits[i] = [];
        sortedTurnarounds[i] = [];
        for (j = 0; j < dispatches.length; j++) {
            sortedWaits[i].push(waits[i * dispatches.length + j]);
            sortedTurnarounds[i].push(turnarounds[i * dispatches.length + j]);
        }
    }
}

// construct charts using data
function buildChart() {

    Chart.defaults.global.animation.duration = 2000;
    Chart.defaults.global.legend.position = "right";
    Chart.defaults.global.title.display = true;

    if (DISPLAY_TURN) {
        let turnaroundCanvas = document.getElementById("turnaroundTimes");
        turnaroundChart = new Chart(turnaroundCanvas, {
            type: 'line',
            data: {
                labels: dispatches,
                datasets: [{
                    label: "Quantum = " + quantums[0],
                    fill: false,
                    borderColor: "orange",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "orange",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedTurnarounds[0],
                },{
                    label: "Quantum = " + quantums[1],
                    fill: false,
                    borderColor: "red",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "red",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedTurnarounds[1],
                },{
                    label: "Quantum = " + quantums[2],
                    fill: false,
                    borderColor: "green",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "green",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedTurnarounds[2],
                },{
                    label: "Quantum = " + quantums[3],
                    fill: false,
                    borderColor: "blue",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "blue",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedTurnarounds[3],
                },{
                    label: "Quantum = " + quantums[4],
                    fill: false,
                    borderColor: "purple",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "purple",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedTurnarounds[4],
                }
                ]
            },
            options: {
                title: {
                    text: "Round-robin CPU Scheduler -- 2000 tasks, 8835 seed"
                },
                scales: {
                    yAxes: [{
                        scaleLabel: {
                            labelString: "Turnaround Time (ticks)",
                            display: true,
                        },
                    }],
                    xAxes: [{
                        scaleLabel: {
                            labelString: "Dispatch Time (ticks)",
                            display: true,
                        },
                    }],
                },
                maintainAspectRatio: false,
            }
        });
    }

    if (DISPLAY_WAIT) {
        let waitCanvas = document.getElementById("waitTimes");
        waitChart = new Chart(waitCanvas, {
            type: 'line',
            data: {
                labels: dispatches,
                datasets: [{
                    label: "Quantum = " + quantums[0],
                    fill: false,
                    borderColor: "orange",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "orange",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedWaits[0],
                },{
                    label: "Quantum = " + quantums[1],
                    fill: false,
                    borderColor: "red",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 0,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "red",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedWaits[1],
                },{
                    label: "Quantum = " + quantums[2],
                    fill: false,
                    borderColor: "green",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "green",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedWaits[2],
                },{
                    label: "Quantum = " + quantums[3],
                    fill: false,
                    borderColor: "blue",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "blue",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedWaits[3],
                },{
                    label: "Quantum = " + quantums[4],
                    fill: false,
                    borderColor: "purple",
                    pointBorderColor: "#888",
                    pointBackgroundColor: "#fff",
                    pointBorderWidth: 1,
                    pointHoverRadius: 5,
                    pointHoverBackgroundColor: "purple",
                    pointHoverBorderColor: "#DDD",
                    pointHoverBorderWidth: 2,
                    pointRadius: 4,
                    pointHitRadius: 10,
                    data: sortedWaits[4],
                }
                ]
            },
            options: {
                title: {
                    text: "Round-robin CPU Scheduler -- 2000 tasks, 8835 seed",
                },
                scales: {
                    yAxes: [{
                        scaleLabel: {
                            labelString: "Wait Time (ticks)",
                            display: true,
                        },
                    }],
                    xAxes: [{
                        scaleLabel: {
                            labelString: "Dispatch Time (ticks)",
                            display: true,
                        },
                    }],
                },
                maintainAspectRatio: false,
            }
        });
    }
}