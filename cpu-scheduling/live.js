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
    rawData = [
        "quantum dispatch wait turnaround",
        "50 0 494290 494813",
        "50 5 556572 557095",
        "50 10 634100 634623",
        "50 15 711373 711896",
        "50 20 788414 788936",
        "50 25 865242 865765",
        "100 0 477710 478233",
        "100 5 509435 509958",
        "100 10 549019 549542",
        "100 15 588605 589127",
        "100 20 628081 628604",
        "100 25 667507 668030",
        "250 0 448112 448635",
        "250 5 461625 462148",
        "250 10 478474 478997",
        "250 15 495304 495826",
        "250 20 512087 512609",
        "250 25 528851 529374",
        "500 0 406189 406711",
        "500 5 413519 414041",
        "500 10 422675 423198",
        "500 15 431827 432349",
        "500 20 440982 441505",
        "500 25 450123 450645",
        "1000 0 322927 323449",
        "1000 5 326929 327451",
        "1000 10 331931 332454",
        "1000 15 336934 337456",
        "1000 20 341936 342459",
        "1000 25 346939 347461"
    ]
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