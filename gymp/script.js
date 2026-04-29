const API_BASE = `http://${window.location.hostname}:4000/api`;
const navLinks = document.querySelectorAll('.sidebar-nav a');
const projectIndicators = document.querySelectorAll('.project-indicators span');
const commandText = document.querySelector('.hero .command');
const panel = document.querySelector('.panel');
const body = document.body;

const commandItems = [
  'Cyber Security',
  'Front-end Development',
  'Creative UI',
  'Modern Gym Dashboard'
];
let commandIndex = 0;
let charIndex = 0;
let typingForward = true;

function updateActiveNav(link) {
  navLinks.forEach(item => item.classList.remove('active'));
  link.classList.add('active');
}

function setActiveIndicator(index) {
  projectIndicators.forEach((dot, dotIndex) => {
    dot.classList.toggle('active', dotIndex === index);
  });
}

function typeCommand() {
  if (!commandText) return;

  const text = commandItems[commandIndex];
  if (typingForward) {
    charIndex += 1;
  } else {
    charIndex -= 1;
  }

  commandText.textContent = text.slice(0, charIndex);

  if (typingForward && charIndex === text.length) {
    typingForward = false;
    setTimeout(typeCommand, 1300);
    return;
  }

  if (!typingForward && charIndex === 0) {
    typingForward = true;
    commandIndex = (commandIndex + 1) % commandItems.length;
    setTimeout(typeCommand, 300);
    return;
  }

  setTimeout(typeCommand, typingForward ? 90 : 40);
}

function createFloatingDots(amount = 12) {
  for (let i = 0; i < amount; i += 1) {
    const dot = document.createElement('span');
    dot.className = 'floating-dot';
    dot.style.left = `${Math.random() * 100}%`;
    dot.style.top = `${Math.random() * 100}%`;
    dot.style.width = `${8 + Math.random() * 16}px`;
    dot.style.height = dot.style.width;
    dot.style.animationDuration = `${8 + Math.random() * 10}s`;
    dot.style.animationDelay = `${-Math.random() * 8}s`;
    body.appendChild(dot);
  }
}

function createCursorGlow() {
  const glow = document.createElement('div');
  glow.className = 'cursor-glow';
  body.appendChild(glow);

  window.addEventListener('mousemove', event => {
    glow.style.transform = `translate3d(${event.clientX}px, ${event.clientY}px, 0)`;
  });
}

function updatePanelStatus() {
  if (!panel) return;

  const time = new Date();
  const hours = String(time.getHours()).padStart(2, '0');
  const minutes = String(time.getMinutes()).padStart(2, '0');
  const statuses = ['ONLINE', 'ACTIVE', 'CONNECTED', 'STABLE'];
  const randomStatus = statuses[Math.floor(Math.random() * statuses.length)];

  panel.innerHTML = `
    <strong>OS: JIHED_OS_V5.0</strong>
    <span><span>SECTOR:</span><span>HOME</span></span>
    <span><span>TIME:</span><span>${hours}:${minutes}</span></span>
    <span><span>SYNC:</span><span>0%</span></span>
    <span><span>STATUS:</span><span>${randomStatus}</span></span>
  `;
}

function revealOnScroll() {
  const elements = document.querySelectorAll('.hero, .topbar, .actions a, .panel, .project-card, .project-header');
  if (!elements.length) return;

  const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        entry.target.classList.add('reveal-visible');
        observer.unobserve(entry.target);
      }
    });
  }, {
    rootMargin: '0px 0px -120px 0px',
    threshold: 0.15
  });

  elements.forEach(element => {
    element.classList.add('reveal-hidden');
    observer.observe(element);
  });
}

function getRequestOptions(method = 'GET', body = null) {
  const options = { method, headers: {} };
  if (body) {
    options.headers['Content-Type'] = 'application/json';
    options.body = JSON.stringify(body);
  }
  return options;
}

async function fetchJSON(url, options = {}) {
  const response = await fetch(url, options);
  if (!response.ok) {
    const errorText = await response.text();
    throw new Error(`${response.status} ${response.statusText}: ${errorText}`);
  }
  return response.json();
}

async function fetchSummary() {
  return fetchJSON(`${API_BASE}/summary`);
}

async function fetchClients() {
  return fetchJSON(`${API_BASE}/clients`);
}

async function fetchClientById(id) {
  return fetchJSON(`${API_BASE}/clients/${id}`);
}

async function createClient(clientData) {
  return fetchJSON(`${API_BASE}/clients`, getRequestOptions('POST', clientData));
}

async function deleteClient(id) {
  return fetchJSON(`${API_BASE}/clients/${id}`, getRequestOptions('DELETE'));
}

async function fetchTrainers() {
  return fetchJSON(`${API_BASE}/trainers`);
}

async function fetchTrainerById(id) {
  return fetchJSON(`${API_BASE}/trainers/${id}`);
}

async function createTrainer(trainerData) {
  return fetchJSON(`${API_BASE}/trainers`, getRequestOptions('POST', trainerData));
}

async function deleteTrainer(id) {
  return fetchJSON(`${API_BASE}/trainers/${id}`, getRequestOptions('DELETE'));
}

async function fetchPresences() {
  return fetchJSON(`${API_BASE}/presences`);
}

async function createPresence(presenceData) {
  return fetchJSON(`${API_BASE}/presences`, getRequestOptions('POST', presenceData));
}

function formatPresenceDate(isoString) {
  const date = new Date(isoString);
  return date.toLocaleDateString('fr-FR');
}

function formatPresenceTime(isoString) {
  const date = new Date(isoString);
  return date.toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit' });
}

async function populatePresenceTrainerOptions() {
  const trainerSelect = document.getElementById('presenceTrainerSelect');
  if (!trainerSelect) return;

  try {
    const trainers = await fetchTrainers();
    trainerSelect.innerHTML = `
      <option value="">Choisir un entraîneur...</option>
      ${trainers.map(trainer => `<option value="${trainer.id}">${trainer.name}</option>`).join('')}
    `;
  } catch (error) {
    console.error('Unable to load trainer list for presence page:', error);
  }
}

async function populatePresenceHistory() {
  const tbody = document.getElementById('presenceHistoryBody');
  if (!tbody) return;

  try {
    const [presences, trainers] = await Promise.all([fetchPresences(), fetchTrainers()]);
    const trainerMap = Object.fromEntries(trainers.map(trainer => [trainer.id, trainer.name]));

    tbody.innerHTML = presences.map(record => {
      const trainerName = trainerMap[record.personId] || 'Inconnu';
      const dateText = formatPresenceDate(record.eventTime);
      const timeText = formatPresenceTime(record.eventTime);
      const eventType = record.eventType || 'Inconnu';
      const statusClass = eventType.toLowerCase() === 'entrée' ? 'success' : 'danger';

      return `
        <tr>
          <td><strong>${trainerName}</strong></td>
          <td>${dateText}</td>
          <td>${timeText}</td>
          <td>${eventType}</td>
          <td><span class="status-badge ${statusClass}">${eventType}</span></td>
        </tr>
      `;
    }).join('');
  } catch (error) {
    console.error('Unable to load presence history:', error);
  }
}

async function updatePresenceSummary() {
  const presentCountNode = document.getElementById('presentCountValue');
  const todayEventsNode = document.getElementById('todayEventsValue');
  if (!presentCountNode && !todayEventsNode) return;

  try {
    const presences = await fetchPresences();
    const today = new Date().toLocaleDateString('fr-FR');
    const todaysRecords = presences.filter(record => formatPresenceDate(record.eventTime) === today);
    const lastEventByTrainer = {};

    presences.forEach(record => {
      lastEventByTrainer[record.personId] = record;
    });

    const presentCount = Object.values(lastEventByTrainer).filter(record => record.eventType.toLowerCase() === 'entrée').length;
    const todayCount = todaysRecords.length;

    if (presentCountNode) presentCountNode.textContent = presentCount.toString();
    if (todayEventsNode) todayEventsNode.textContent = todayCount.toString();
  } catch (error) {
    console.error('Unable to update presence summary:', error);
  }
}

async function setupPresencesPage() {
  const trainerSelect = document.getElementById('presenceTrainerSelect');
  const eventTypeSelect = document.getElementById('presenceEventType');
  const saveButton = document.getElementById('savePresenceButton');
  if (!trainerSelect || !eventTypeSelect || !saveButton) return;

  await populatePresenceTrainerOptions();
  await populatePresenceHistory();
  await updatePresenceSummary();

  saveButton.addEventListener('click', async () => {
    const trainerId = trainerSelect.value;
    const eventType = eventTypeSelect.value;

    if (!trainerId || !eventType) {
      alert('Veuillez sélectionner un entraîneur et un type d\'événement.');
      return;
    }

    try {
      await createPresence({
        personId: trainerId,
        personType: 'trainer',
        eventType,
        eventTime: new Date().toISOString(),
        notes: `Enregistrer ${eventType}`
      });
      await populatePresenceHistory();
      await updatePresenceSummary();
    } catch (error) {
      console.error('Unable to save presence record:', error);
      alert('Erreur lors de l\'enregistrement de la présence.');
    }
  });
}

function setClientCount(count) {
  const countLabel = document.getElementById('clientsCount');
  if (countLabel) {
    countLabel.textContent = `Liste des Clients (${count})`;
  }
}

function setTrainerCount(count) {
  const heading = document.getElementById('trainerCountHeading');
  if (heading) {
    heading.textContent = `Équipe actuelle (${count})`;
  }
}

function getStatusClass(status) {
  const normalized = status ? status.toString().toLowerCase() : '';
  return normalized === 'actif' || normalized === 'active' || normalized === 'نشط' || normalized === 'نشطة' ? 'success' : 'danger';
}

function formatSummaryValue(value) {
  return value != null ? value : '0';
}

async function updateDashboardSummary() {
  const totalClients = document.getElementById('totalClientsValue');
  const totalTrainers = document.getElementById('totalTrainersValue');
  const activeSubscriptions = document.getElementById('activeSubscriptionsValue');
  const netProfit = document.getElementById('netProfitValue');

  if (!totalClients && !totalTrainers && !activeSubscriptions && !netProfit) {
    return;
  }

  try {
    const summary = await fetchSummary();
    if (totalClients) totalClients.textContent = formatSummaryValue(summary.totalClients);
    if (totalTrainers) totalTrainers.textContent = formatSummaryValue(summary.totalTrainers);
    if (activeSubscriptions) activeSubscriptions.textContent = formatSummaryValue(summary.activeSubscriptions);
    if (netProfit && netProfit.textContent.trim() === '0') netProfit.textContent = '24.5k';
  } catch (error) {
    console.warn('Unable to load dashboard summary:', error);
  }
}

async function populateClientTable() {
  const tbody = document.getElementById('clientsTableBody');
  if (!tbody) return;

  try {
    const clients = await fetchClients();
    tbody.innerHTML = clients.map(client => {
      const now = new Date();
      const endDate = new Date(client.endDate);
      const active = endDate >= now;
      const statusClass = active ? 'success' : 'danger';
      const statusText = active ? 'Actif' : 'Expiré';

      return `
        <tr data-id="${client.id}">
          <td><strong>${client.firstName} ${client.lastName}</strong></td>
          <td>${client.phone}</td>
          <td>${client.subscriptionLabel}</td>
          <td>${client.startDate}</td>
          <td>${client.endDate}</td>
          <td>${client.price} TND</td>
          <td><span class="status-badge ${statusClass}">${statusText}</span></td>
          <td class="actions-cell">
            <button class="icon-small" data-action="edit-client" data-id="${client.id}">✏️</button>
            <button class="icon-small danger" data-action="delete-client" data-id="${client.id}">🗑️</button>
          </td>
        </tr>
      `;
    }).join('');

    setClientCount(clients.length);
  } catch (error) {
    console.error('Error loading clients:', error);
  }
}

async function populateTrainerTable() {
  const tbody = document.getElementById('trainersTableBody');
  if (!tbody) return;

  try {
    const trainers = await fetchTrainers();
    tbody.innerHTML = trainers.map(trainer => `
      <tr data-id="${trainer.id}">
        <td>
          <button class="trainer-profile-trigger" data-trainer="${trainer.id}">
            <span class="trainer-avatar">${trainer.name.trim().charAt(0).toUpperCase()}</span>
            <strong>${trainer.name}</strong>
          </button>
        </td>
        <td>${trainer.role}</td>
        <td>${trainer.phone}</td>
        <td>${trainer.rate}</td>
        <td><span class="status-badge ${getStatusClass(trainer.status)}">${trainer.status}</span></td>
        <td class="actions-cell">
          <button class="icon-small" data-action="edit-trainer" data-id="${trainer.id}">✏️</button>
          <button class="icon-small danger" data-action="delete-trainer" data-id="${trainer.id}">🗑️</button>
        </td>
      </tr>
    `).join('');

    setTrainerCount(trainers.length);
  } catch (error) {
    console.error('Error loading trainers:', error);
  }
}

async function openTrainerModal(trainerId) {
  const overlay = document.getElementById('trainerModal');
  if (!overlay) return;

  let profile = null;
  try {
    profile = await fetchTrainerById(trainerId);
  } catch (error) {
    console.warn('Trainer profile not found via API, falling back to static data');
  }

  const defaultProfiles = {
    'karim-hamdi': {
      name: 'كريم حمدي',
      role: 'Coach Fitness',
      rate: '6 TND/heure',
      phone: '88765432',
      status: 'Actif',
      bio: 'مدرب لياقة ذو خبرة واسعة في برامج القوة و اللياقة الجماعية. يعمل على تصميم خطط تدريبية مخصصة لكل هدف ويهتم بمتابعة الأداء اليومي.',
      certificates: [
        'دبلوم مدرب معتمد في اللياقة الرياضية',
        'شهادة التغذية الرياضية',
        'دورة تدريب EMS متقدمة'
      ],
      specialties: ['تقوية الجسم', 'بناء العضلات', 'تدريب المجموعات'],
      schedule: [
        { day: 'الإثنين', time: '08:00 - 15:00' },
        { day: 'الأربعاء', time: '10:00 - 17:00' },
        { day: 'الجمعة', time: '08:00 - 14:00' }
      ]
    },
    'salma-talbi': {
      name: 'سلمى الطالبي',
      role: 'Coach Cardio',
      rate: '6 TND/heure',
      phone: '87654321',
      status: 'Active',
      bio: 'متخصصة في تمارين القلب والأوعية الدموية وبرامج اللياقة السريعة. تساعد الأعضاء على تحسين القدرة على التحمل وخفض الوزن بطريقة آمنة.',
      certificates: [
        'اعتماد تدريب الكارديو',
        'شهادة الصحة العامة',
        'دورة تدريب القلب الرياضي'
      ],
      specialties: ['كارديو', 'حرق الدهون', 'المجموعات الصباحية'],
      schedule: [
        { day: 'الثلاثاء', time: '09:00 - 16:00' },
        { day: 'الخميس', time: '09:00 - 16:00' },
        { day: 'السبت', time: '10:00 - 14:00' }
      ]
    }
  };

  const profileData = profile || defaultProfiles[trainerId];
  if (!profileData) return;

  document.getElementById('trainerModalTitle').textContent = profileData.name;
  document.getElementById('trainerModalRole').textContent = profileData.role || 'Entraîneur';
  document.getElementById('trainerModalStatus').textContent = profileData.status ? `Statut : ${profileData.status}` : '';
  document.getElementById('trainerModalRate').textContent = profileData.rate || '';
  document.getElementById('trainerModalPhone').textContent = profileData.phone || '';
  document.getElementById('trainerModalBio').textContent = profileData.bio || 'Informations de l\'entraîneur non disponibles.';

  const certList = document.getElementById('trainerCertificates');
  if (certList) certList.innerHTML = (profileData.certificates || []).map(cert => `<li>${cert}</li>`).join('');

  const specialties = document.getElementById('trainerSpecialties');
  if (specialties) specialties.innerHTML = (profileData.specialties || []).map(item => `<span>${item}</span>`).join('');

  const scheduleBody = document.getElementById('trainerSchedule');
  if (scheduleBody) {
    scheduleBody.innerHTML = (profileData.schedule || []).map(row => `
      <tr>
        <td>${row.day}</td>
        <td>${row.time}</td>
      </tr>
    `).join('');
  }

  overlay.classList.add('open');
  overlay.setAttribute('aria-hidden', 'false');
}

function closeTrainerModal() {
  const overlay = document.getElementById('trainerModal');
  if (!overlay) return;
  overlay.classList.remove('open');
  overlay.setAttribute('aria-hidden', 'true');
}

function openClientModal() {
  const overlay = document.getElementById('clientModal');
  if (!overlay) return;
  overlay.classList.add('open');
  overlay.setAttribute('aria-hidden', 'false');
}

function closeClientModal() {
  const overlay = document.getElementById('clientModal');
  if (!overlay) return;
  overlay.classList.remove('open');
  overlay.setAttribute('aria-hidden', 'true');
}

function updateClientPrice() {
  const typeSelect = document.getElementById('clientSubscriptionType');
  const priceLabel = document.getElementById('clientPrice');
  if (!typeSelect || !priceLabel) return;
  const price = typeSelect.value === 'three' ? 200 : typeSelect.value === 'six' ? 380 : 75;
  priceLabel.textContent = `${price} TND`;
}

async function setupClientPage() {
  const openButton = document.getElementById('openClientModal');
  const closeButton = document.getElementById('closeClientModal');
  const cancelButton = document.getElementById('cancelClient');
  const overlay = document.getElementById('clientModal');
  const form = document.getElementById('clientForm');
  const typeSelect = document.getElementById('clientSubscriptionType');
  const tbody = document.getElementById('clientsTableBody');

  if (!form || !tbody) return;

  if (openButton) openButton.addEventListener('click', openClientModal);
  if (closeButton) closeButton.addEventListener('click', closeClientModal);
  if (cancelButton) cancelButton.addEventListener('click', closeClientModal);
  if (overlay) {
    overlay.addEventListener('click', event => {
      if (event.target === overlay) closeClientModal();
    });
  }
  if (typeSelect) typeSelect.addEventListener('change', updateClientPrice);

  form.addEventListener('submit', async event => {
    event.preventDefault();
    const firstName = document.getElementById('clientFirstName').value.trim();
    const lastName = document.getElementById('clientLastName').value.trim();
    const phone = document.getElementById('clientPhone').value.trim();
    const subscriptionType = document.getElementById('clientSubscriptionType').value;
    const startDate = document.getElementById('clientStartDate').value;
    const endDate = document.getElementById('clientEndDate').value;

    if (!firstName || !lastName || !phone || !startDate || !endDate) return;

    const label = subscriptionType === 'three' ? 'Trois mois' : subscriptionType === 'six' ? 'Six mois' : 'Mensuel';
    const price = subscriptionType === 'three' ? 200 : subscriptionType === 'six' ? 380 : 75;

    try {
      await createClient({
        firstName,
        lastName,
        phone,
        subscriptionType,
        subscriptionLabel: label,
        startDate,
        endDate,
        price
      });
      await populateClientTable();
      await updateDashboardSummary();
      form.reset();
      updateClientPrice();
      closeClientModal();
    } catch (error) {
      console.error('Unable to create client:', error);
      alert(`Erreur lors de la création du client : ${error.message}`);
    }
  });

  tbody.addEventListener('click', async event => {
    const deleteButton = event.target.closest('button[data-action="delete-client"]');
    if (!deleteButton) return;
    const clientId = deleteButton.dataset.id;
    if (!clientId) return;

    try {
      await deleteClient(clientId);
      await populateClientTable();
      await updateDashboardSummary();
    } catch (error) {
      console.error('Unable to delete client:', error);
      alert('Erreur lors de la suppression du client.');
    }
  });

  await populateClientTable();
}

async function setupTrainerPage() {
  const addButton = document.getElementById('openTrainerAddModal');
  const addModal = document.getElementById('trainerAddModal');
  const closeAddButton = document.getElementById('closeTrainerAddModal');
  const cancelAddButton = document.getElementById('cancelTrainer');
  const trainerForm = document.getElementById('trainerForm');
  const trainerTable = document.getElementById('trainersTableBody');

  if (!trainerForm || !trainerTable) return;

  if (addButton) {
    addButton.addEventListener('click', () => {
      if (addModal) {
        addModal.classList.add('open');
        addModal.setAttribute('aria-hidden', 'false');
      }
    });
  }
  if (closeAddButton) {
    closeAddButton.addEventListener('click', () => {
      if (addModal) {
        addModal.classList.remove('open');
        addModal.setAttribute('aria-hidden', 'true');
      }
    });
  }
  if (cancelAddButton) {
    cancelAddButton.addEventListener('click', () => {
      if (addModal) {
        addModal.classList.remove('open');
        addModal.setAttribute('aria-hidden', 'true');
      }
    });
  }
  if (addModal) {
    addModal.addEventListener('click', event => {
      if (event.target === addModal) {
        addModal.classList.remove('open');
        addModal.setAttribute('aria-hidden', 'true');
      }
    });
  }

  trainerForm.addEventListener('submit', async event => {
    event.preventDefault();
    const name = document.getElementById('trainerName').value.trim();
    const role = document.getElementById('trainerRole').value.trim();
    const phone = document.getElementById('trainerPhone').value.trim();
    const rate = document.getElementById('trainerRate').value.trim();
    const status = document.getElementById('trainerStatus').value;

    if (!name || !role || !phone || !rate) return;

    try {
      await createTrainer({ name, role, phone, rate, status });
      await populateTrainerTable();
      await updateDashboardSummary();
      trainerForm.reset();
      if (addModal) {
        addModal.classList.remove('open');
        addModal.setAttribute('aria-hidden', 'true');
      }
    } catch (error) {
      console.error('Unable to create trainer:', error);
      alert('Erreur lors de l\'ajout du coach.');
    }
  });

  trainerTable.addEventListener('click', async event => {
    const deleteButton = event.target.closest('button[data-action="delete-trainer"]');
    if (!deleteButton) return;
    const trainerId = deleteButton.dataset.id;
    if (!trainerId) return;

    try {
      await deleteTrainer(trainerId);
      await populateTrainerTable();
      await updateDashboardSummary();
    } catch (error) {
      console.error('Unable to delete trainer:', error);
      alert('Erreur lors de la suppression du coach.');
    }
  });

  await populateTrainerTable();
}

function setupTrainerModal() {
  const closeBtn = document.getElementById('closeTrainerModal');

  document.body.addEventListener('click', event => {
    const trigger = event.target.closest('.trainer-profile-trigger');
    if (trigger) {
      const trainerId = trigger.dataset.trainer;
      openTrainerModal(trainerId);
      return;
    }

    if (event.target === closeBtn || event.target.id === 'trainerModal') {
      closeTrainerModal();
    }
  });

  document.body.addEventListener('keydown', event => {
    if (event.key === 'Escape') closeTrainerModal();
  });
}

async function initializePage() {
  await updateDashboardSummary();
  setupTrainerModal();
  await setupTrainerPage();
  await setupClientPage();
  await setupPresencesPage();
}

navLinks.forEach(link => {
  link.addEventListener('click', event => {
    updateActiveNav(link);
    if (link.getAttribute('href') && link.getAttribute('href').startsWith('#')) {
      event.preventDefault();
      const targetId = link.getAttribute('href').substring(1);
      const target = document.getElementById(targetId);
      if (target) target.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  });
});

let currentIndicator = 0;
if (projectIndicators.length) {
  setActiveIndicator(currentIndicator);
  setInterval(() => {
    currentIndicator = (currentIndicator + 1) % projectIndicators.length;
    setActiveIndicator(currentIndicator);
  }, 3000);
}

document.addEventListener('DOMContentLoaded', async () => {
  if (commandText) {
    charIndex = 0;
    typingForward = true;
    typeCommand();
  }

  createFloatingDots(14);
  createCursorGlow();
  updatePanelStatus();
  setInterval(updatePanelStatus, 10000);
  revealOnScroll();
  await initializePage();
});
