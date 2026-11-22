/* ===== Train Ticket Reservation - web version of reservation.c ===== */

const trains = [
  { no: 12904, name: "Golden Temple Mail", from: "Amritsar", to: "Mumbai", fare: [765, 2010, 2925] },
  { no: 12625, name: "Kerala Express", from: "Trivandrum", to: "New Delhi", fare: [905, 2350, 3420] },
  { no: 12839, name: "Howrah Mail", from: "Howrah", to: "Chennai", fare: [720, 1890, 2715] },
  { no: 12417, name: "Prayagraj Express", from: "Prayagraj", to: "New Delhi", fare: [395, 1045, 1480] },
  { no: 12715, name: "Sachkhand Express", from: "Nanded", to: "Amritsar", fare: [840, 2205, 3180] }
];

const classNames = ["Sleeper (SL)", "AC 3 Tier (3A)", "AC 2 Tier (2A)"];
const coach = ["S1", "B1", "A1"];
const berthType = ["LB", "MB", "UB", "LB", "MB", "UB", "SL", "SU"];
const cancelCharge = [120, 180, 200];

const BERTHS = 16;
const WL_LIMIT = 6;

let tickets = JSON.parse(localStorage.getItem("railTickets")) || [];

function saveTickets() {
  localStorage.setItem("railTickets", JSON.stringify(tickets));
}


/* ---------- sidebar navigation ---------- */
const navButtons = document.querySelectorAll(".nav-btn");

navButtons.forEach(btn => {
  btn.addEventListener("click", () => openPage(btn.dataset.page));
});

function openPage(id) {
  document.querySelectorAll(".page").forEach(p => p.classList.add("hidden"));
  document.getElementById(id).classList.remove("hidden");

  navButtons.forEach(b => b.classList.remove("active"));
  document.querySelector(`.nav-btn[data-page="${id}"]`).classList.add("active");
}


/* ---------- helper functions ---------- */
const findTrain = no => trains.find(t => t.no == no);

// date input gives yyyy-mm-dd but C program uses dd-mm-yyyy
function changeFormat(d) {
  const parts = d.split("-");
  return `${parts[2]}-${parts[1]}-${parts[0]}`;
}

function berthName(cls, berth) {
  return `${coach[cls]}/${berth}/${berthType[(berth - 1) % 8]}`;
}

function message(type, text) {
  return `<div class="msg ${type}">${text}</div>`;
}

// booked berths and waiting list count for one train, class and date
function getStatus(trainNo, cls, date) {
  const booked = new Array(BERTHS + 1).fill(false);
  let wl = 0;

  tickets.forEach(t => {
    if (t.trainNo == trainNo && t.cls == cls && t.date == date) {
      t.passengers.forEach(p => {
        if (p.berth == 0) wl++;
        else booked[p.berth] = true;
      });
    }
  });

  let free = 0;
  for (let i = 1; i <= BERTHS; i++) {
    if (!booked[i]) free++;
  }
  return { booked, wl, free };
}

// current WL number of passenger k of a ticket
function wlNumber(ticket, k) {
  let pos = 0;
  for (const t of tickets) {
    if (t.trainNo == ticket.trainNo && t.cls == ticket.cls && t.date == ticket.date) {
      for (let i = 0; i < t.passengers.length; i++) {
        if (t.passengers[i].berth == 0) {
          pos++;
          if (t.pnr == ticket.pnr && i == k) return pos;
        }
      }
    }
  }
  return 0;
}

function ticketHTML(t) {
  const train = findTrain(t.trainNo);
  let rows = "";

  t.passengers.forEach((p, i) => {
    const status = p.berth == 0
      ? `<span class="tag wl">WL ${wlNumber(t, i)}</span>`
      : `<span class="tag cnf">CNF ${berthName(t.cls, p.berth)}</span>`;
    rows += `<tr><td>${i + 1}</td><td>${p.name}</td><td>${p.age}</td><td>${p.gender}</td><td>${status}</td></tr>`;
  });

  return `
    <div class="ticket">
      <div class="ticket-head">
        <span>PNR: <b>${t.pnr}</b></span>
        <span>${t.date}</span>
      </div>
      <div class="ticket-body">
        <p><b>${train.no} - ${train.name}</b></p>
        <p>${train.from} &rarr; ${train.to} &nbsp;|&nbsp; ${classNames[t.cls]}</p>
        <table>
          <tr><th>No</th><th>Name</th><th>Age</th><th>Sex</th><th>Status</th></tr>
          ${rows}
        </table>
        <p class="total">Total Fare: Rs ${t.amount}</p>
      </div>
    </div>`;
}


/* ---------- trains page ---------- */
function loadTrains() {
  let cards = "";
  let options = `<option value="">-- select train --</option>`;

  trains.forEach(t => {
    cards += `
      <div class="card">
        <div class="no">${t.no}</div>
        <h4>${t.name}</h4>
        <div class="route">${t.from} &rarr; ${t.to}</div>
        <div class="fares">
          <span>SL: Rs ${t.fare[0]}</span>
          <span>3A: Rs ${t.fare[1]}</span>
          <span>2A: Rs ${t.fare[2]}</span>
        </div>
      </div>`;
    options += `<option value="${t.no}">${t.no} - ${t.name}</option>`;
  });

  document.getElementById("trainCards").innerHTML = cards;
  document.querySelectorAll(".train-select").forEach(s => (s.innerHTML = options));
}


/* ---------- availability ---------- */
document.getElementById("availBtn").addEventListener("click", () => {
  const trainNo = document.getElementById("aTrain").value;
  const d = document.getElementById("aDate").value;
  const out = document.getElementById("availResult");

  if (trainNo == "" || d == "") {
    out.innerHTML = message("err", "Select train and date first");
    return;
  }

  const date = changeFormat(d);
  let html = `<div class="avail-boxes">`;

  for (let c = 0; c < 3; c++) {
    const s = getStatus(trainNo, c, date);
    let state;
    if (s.free > 0) state = `<div class="state green">AVAILABLE ${s.free}</div>`;
    else if (s.wl < WL_LIMIT) state = `<div class="state orange">WL ${s.wl + 1}</div>`;
    else state = `<div class="state red">REGRET</div>`;

    html += `<div class="avail-box"><div class="cls">${classNames[c]}</div>${state}</div>`;
  }
  html += `</div>`;
  out.innerHTML = html;
});


/* ---------- booking ---------- */
function makePassengerForms() {
  const n = parseInt(document.getElementById("bCount").value);
  let html = "";
  for (let i = 0; i < n; i++) {
    html += `
      <div class="passenger">
        <p>Passenger ${i + 1}</p>
        <div class="form-row">
          <div><label>Name</label><input type="text" id="name${i}" maxlength="29"></div>
          <div><label>Age</label><input type="number" id="age${i}" min="1" max="120"></div>
          <div>
            <label>Gender</label>
            <select id="gender${i}">
              <option value="M">Male</option>
              <option value="F">Female</option>
              <option value="O">Other</option>
            </select>
          </div>
        </div>
      </div>`;
  }
  document.getElementById("passengerBox").innerHTML = html;
}

document.getElementById("bCount").addEventListener("change", makePassengerForms);

document.getElementById("bookBtn").addEventListener("click", () => {
  const out = document.getElementById("bookResult");
  const trainNo = document.getElementById("bTrain").value;
  const d = document.getElementById("bDate").value;
  const cls = parseInt(document.getElementById("bClass").value);
  const count = parseInt(document.getElementById("bCount").value);

  if (trainNo == "" || d == "") {
    out.innerHTML = message("err", "Please select train and date");
    return;
  }

  const passengers = [];
  for (let i = 0; i < count; i++) {
    const name = document.getElementById("name" + i).value.trim();
    const age = parseInt(document.getElementById("age" + i).value);
    const gender = document.getElementById("gender" + i).value;

    if (name == "" || isNaN(age) || age < 1 || age > 120) {
      out.innerHTML = message("err", `Please check details of passenger ${i + 1}`);
      return;
    }
    passengers.push({ name: name, age: age, gender: gender, berth: 0 });
  }

  const date = changeFormat(d);
  const s = getStatus(trainNo, cls, date);

  if (count > s.free + (WL_LIMIT - s.wl)) {
    out.innerHTML = message("err", "REGRET! Not enough berths or waiting list left.");
    return;
  }

  // berth allotment, seniors get lower berth first
  passengers.forEach(p => {
    if (p.age >= 60) {
      for (let j = 1; j <= BERTHS; j++) {
        if (!s.booked[j] && berthType[(j - 1) % 8] == "LB") {
          p.berth = j;
          break;
        }
      }
    }
    if (p.berth == 0) {
      for (let j = 1; j <= BERTHS; j++) {
        if (!s.booked[j]) {
          p.berth = j;
          break;
        }
      }
    }
    if (p.berth != 0) s.booked[p.berth] = true;
  });

  let total = findTrain(trainNo).fare[cls] * count;
  if (cls > 0) total = total + Math.floor(total * 5 / 100);   // 5% GST on AC

  if (!confirm(`Total fare is Rs ${total}. Confirm booking?`)) {
    out.innerHTML = message("err", "Booking not done.");
    return;
  }

  let pnr;
  do {
    pnr = 100000 + Math.floor(Math.random() * 900000);
  } while (tickets.some(t => t.pnr == pnr));

  const ticket = {
    pnr: pnr,
    trainNo: parseInt(trainNo),
    cls: cls,
    date: date,
    passengers: passengers,
    amount: total
  };
  tickets.push(ticket);
  saveTickets();

  out.innerHTML = message("ok", "Ticket booked!!") + ticketHTML(ticket);
  makePassengerForms();   // clear the passenger forms
});


/* ---------- pnr status ---------- */
document.getElementById("statusBtn").addEventListener("click", () => {
  const pnr = document.getElementById("sPnr").value;
  const out = document.getElementById("statusResult");
  const t = tickets.find(x => x.pnr == pnr);

  out.innerHTML = t ? ticketHTML(t) : message("err", "Invalid PNR!");
});


/* ---------- cancel ---------- */
document.getElementById("cancelBtn").addEventListener("click", () => {
  const pnr = document.getElementById("cPnr").value;
  const out = document.getElementById("cancelResult");
  const index = tickets.findIndex(x => x.pnr == pnr);

  if (index == -1) {
    out.innerHTML = message("err", "Invalid PNR!");
    return;
  }

  const t = tickets[index];
  if (!confirm(`Cancel PNR ${t.pnr} (${t.passengers.length} passenger/s)?`)) {
    return;
  }

  let charge = 0;
  const freed = [];
  t.passengers.forEach(p => {
    if (p.berth == 0) {
      charge += 60;
    } else {
      charge += cancelCharge[t.cls];
      freed.push(p.berth);
    }
  });
  let refund = t.amount - charge;
  if (refund < 0) refund = 0;

  tickets.splice(index, 1);

  // give freed berths to waiting list (first come first serve)
  const promoted = [];
  tickets.forEach(x => {
    if (x.trainNo == t.trainNo && x.cls == t.cls && x.date == t.date) {
      x.passengers.forEach(p => {
        if (p.berth == 0 && freed.length > 0) {
          p.berth = freed.shift();
          promoted.push(`PNR ${x.pnr} - ${p.name} is now CONFIRMED (${berthName(x.cls, p.berth)})`);
        }
      });
    }
  });
  saveTickets();

  let html = message("ok", `Ticket cancelled.<br>Cancellation charges: Rs ${charge}<br>Refund amount: Rs ${refund}`);
  if (promoted.length > 0) {
    html += message("ok", promoted.join("<br>"));
  }
  out.innerHTML = html;
});


/* ---------- chart (admin) ---------- */
document.getElementById("chartBtn").addEventListener("click", () => {
  const pass = document.getElementById("adminPass").value;
  const trainNo = document.getElementById("chTrain").value;
  const d = document.getElementById("chDate").value;
  const out = document.getElementById("chartResult");

  if (pass != "admin123") {
    out.innerHTML = message("err", "Wrong password!");
    return;
  }
  if (trainNo == "" || d == "") {
    out.innerHTML = message("err", "Select train and date");
    return;
  }

  const date = changeFormat(d);
  let html = "";
  let total = 0;

  for (let c = 0; c < 3; c++) {
    let rows = "";
    tickets.forEach(t => {
      if (t.trainNo == trainNo && t.cls == c && t.date == date) {
        t.passengers.forEach((p, i) => {
          const berth = p.berth == 0 ? "WL " + wlNumber(t, i) : berthName(c, p.berth);
          rows += `<tr><td>${t.pnr}</td><td>${p.name}</td><td>${p.age}</td><td>${p.gender}</td><td>${berth}</td></tr>`;
        });
        total += t.amount;
      }
    });
    if (rows == "") rows = `<tr><td colspan="5">No passengers</td></tr>`;

    html += `
      <div class="chart-class">
        <h3>${classNames[c]}</h3>
        <table>
          <tr><th>PNR</th><th>Name</th><th>Age</th><th>Sex</th><th>Berth</th></tr>
          ${rows}
        </table>
      </div>`;
  }
  html += message("ok", `Total collection: Rs ${total}`);
  out.innerHTML = html;
});


/* ---------- on page load ---------- */
loadTrains();
makePassengerForms();

// no booking for past dates
const today = new Date();
const minDate = today.getFullYear() + "-" +
  String(today.getMonth() + 1).padStart(2, "0") + "-" +
  String(today.getDate()).padStart(2, "0");
document.getElementById("bDate").min = minDate;
