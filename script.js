"use strict";

/* =========================
   1. LOADER (FAST & CLEAN)
========================= */
window.addEventListener("DOMContentLoaded", () => {
    const loader = document.getElementById("loader");

    setTimeout(() => {
        loader.style.opacity = "0";
        setTimeout(() => loader.style.display = "none", 500);
        initTypewriter();
    }, 800); // أسرع من قبل
});

/* =========================
   2. CUSTOM CURSOR (FIXED)
========================= */
const cursor = document.querySelector('.cursor');
const trail = document.querySelector('.cursor-trail');

document.addEventListener('mousemove', (e) => {
    cursor.style.transform = `translate(${e.clientX}px, ${e.clientY}px)`;
    trail.style.transform = `translate(${e.clientX - 15}px, ${e.clientY - 15}px)`;
});

// Hover effect
document.querySelectorAll('a, button, .project-card').forEach(el => {
    el.addEventListener('mouseenter', () => {
        cursor.style.transform += " scale(2)";
        cursor.style.background = "#fff";
    });

    el.addEventListener('mouseleave', () => {
        cursor.style.transform = cursor.style.transform.replace(" scale(2)", "");
        cursor.style.background = "var(--primary)";
    });
});

/* =========================
   3. CANVAS BACKGROUND (OPTIMIZED)
========================= */
const canvas = document.getElementById('bg');
const ctx = canvas.getContext('2d');
let particles = [];
let animationId;

function initCanvas() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
}

class Particle {
    constructor() {
        this.reset();
    }

    reset() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.vx = (Math.random() - 0.5) * 0.4;
        this.vy = (Math.random() - 0.5) * 0.4;
        this.size = Math.random() * 1.5;
    }

    update() {
        this.x += this.vx;
        this.y += this.vy;

        if (this.x < 0 || this.x > canvas.width) this.vx *= -1;
        if (this.y < 0 || this.y > canvas.height) this.vy *= -1;
    }

    draw() {
        ctx.fillStyle = "rgba(0, 242, 255, 0.5)";
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
    }
}

function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    particles.forEach(p => {
        p.update();
        p.draw();
    });

    animationId = requestAnimationFrame(animate);
}

// Init
initCanvas();
particles = Array.from({ length: 70 }, () => new Particle());
animate();

window.addEventListener('resize', initCanvas);

/* =========================
   4. TYPEWRITER (PRO VERSION)
========================= */
function initTypewriter() {
    const roles = [
        "Full-Stack Developer",
        "UI/UX Designer",
        "Creative Coder",
        "Problem Solver"
    ];

    const target = document.getElementById('type');
    let roleIndex = 0;
    let charIndex = 0;
    let deleting = false;

    function type() {
        const current = roles[roleIndex];

        if (!deleting) {
            target.textContent = current.substring(0, charIndex++);
        } else {
            target.textContent = current.substring(0, charIndex--);
        }

        if (charIndex === current.length) deleting = true;

        if (charIndex === 0 && deleting) {
            deleting = false;
            roleIndex = (roleIndex + 1) % roles.length;
        }

        setTimeout(type, deleting ? 50 : 100);
    }

    type();
}

/* =========================
   5. HERO PARALLAX
========================= */
const hero = document.querySelector('.hero');

document.addEventListener('mousemove', (e) => {
    const x = (window.innerWidth / 2 - e.clientX) / 30;
    const y = (window.innerHeight / 2 - e.clientY) / 30;

    hero.style.transform = `translate(${x}px, ${y}px)`;
});

/* =========================
   6. SCROLL EFFECTS
========================= */
const sections = document.querySelectorAll('.reveal');
const nav = document.querySelector('nav');

window.addEventListener('scroll', () => {

    // Navbar blur
    if (window.scrollY > 50) nav.classList.add('scrolled');
    else nav.classList.remove('scrolled');

    // Reveal sections
    sections.forEach(section => {
        const top = section.getBoundingClientRect().top;

        if (top < window.innerHeight - 120) {
            section.classList.add('active');
        }
    });
});

/* =========================
   7. YEAR AUTO UPDATE
========================= */
document.getElementById('year').textContent = new Date().getFullYear();

/* =========================
   8. PAUSE ANIMATION (FIXED)
========================= */
document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
        cancelAnimationFrame(animationId);
    } else {
        animate();
    }
});

