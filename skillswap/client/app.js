const API = "http://localhost:3000/api";
let token = localStorage.getItem('token');
let currentUser = null;

// تحميل التطبيق
window.onload = async () => {
    if(token) {
        try { await loadUser(); }
        catch { logout(); }
    }

    setTimeout(() => document.getElementById('loader').style.display = 'none', 800);
    renderPage();
}

// جلب بيانات المستخدم
async function loadUser() {
    const res = await fetch(`${API}/matches`, {
        headers: { 'Authorization': `Bearer ${token}` }
    });
    currentUser = await res.json();
}

// تسجيل الخروج
function logout() {
    localStorage.removeItem('token');
    token = null;
    currentUser = null;
    renderPage();
}

// نظام عرض الصفحات
function renderPage(page = 'home') {
    const app = document.getElementById('app');

    if(!token) return renderAuth(app);
    if(page === 'matches') return renderMatches(app);
    if(page === 'chat') return renderChat(app);
    if(page === 'profile') return renderProfile(app);
    
    renderHome(app);
}

function renderAuth(app) {
    app.innerHTML = `
    <div class="auth-form fade-in">
        <h2>مرحباً بك في SkillSwap AI</h2>
        <p>بدل مهاراتك مع اشخاص متشابهين عن طريق الذكاء الاصطناعي</p>
        
        <input type="email" id="email" placeholder="البريد الالكتروني">
        <input type="password" id="password" placeholder="كلمة المرور">
        
        <button onclick="login()">تسجيل الدخول</button>
        <button style="margin-top: 10px; background: transparent; border: 1px solid var(--border)" onclick="register()">انشاء حساب جديد</button>
    </div>
    `;
}

async function renderMatches(app) {
    const users = await (await fetch(`${API}/matches`, {
        headers: { 'Authorization': `Bearer ${token}` }
    })).json();

    app.innerHTML = `
    <h1>✨ المطابقات المقترحة لك</h1>
    <p>الذكاء الاصطناعي وجد هؤلاء الاشخاص بناء على مهاراتك</p>

    <div class="grid">
        ${users.map(user => `
        <div class="user-card fade-in">
            <span class="match-score">${user.match_score}% مطابقة</span>
            <h3>${user.name}</h3>
            <p>${user.bio}</p>
            <div class="skills">
                ${JSON.parse(user.skills).map(s => `<span class="tag">${s}</span>`).join('')}
            </div>
            <button style="margin-top: 16px" onclick="openChat(${user.id})">بدل محادثة</button>
        </div>
        `).join('')}
    </div>
    `;
}

// باقي دوال الصفحات والوظائف
async function login() {
    const email = document.getElementById('email').value;
    const password = document.getElementById('password').value;

    const res = await fetch(`${API}/login`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({email, password})
    });

    const data = await res.json();
    if(res.ok) {
        token = data.token;
        localStorage.setItem('token', token);
        renderPage();
    }
}

async function register() {
    const email = document.getElementById('email').value;
    const password = document.getElementById('password').value;

    const res = await fetch(`${API}/register`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({
            name: "مستخدم جديد",
            email, password,
            skills: ["تطوير ويب", "تصميم", "برمجة"],
            bio: "أحب تعلم مهارات جديدة"
        })
    });

    const data = await res.json();
    if(res.ok) {
        token = data.token;
        localStorage.setItem('token', token);
        renderPage();
    }
}

// نظام التنقل بين الصفحات
document.querySelectorAll('[data-page]').forEach(link => {
    link.addEventListener('click', (e) => {
        e.preventDefault();
        document.querySelectorAll('[data-page]').forEach(l => l.classList.remove('active'));
        link.classList.add('active');
        renderPage(link.dataset.page);
    })
})

document.getElementById('logout-btn').addEventListener('click', logout);