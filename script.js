"use strict";

const loader = document.getElementById("loader");
const perc = document.getElementById("load-perc");

let p=0;
const int=setInterval(()=>{
p+=2;
perc.textContent=p+"%";
if(p>=100){
clearInterval(int);
setTimeout(()=>loader.style.display="none",500);
}
},30);

/* Cursor */
const c=document.querySelector('.cursor');
const t=document.querySelector('.cursor-trail');

let cursorX = 0, cursorY = 0;
let trailX = 0, trailY = 0;

document.addEventListener('mousemove',e=>{
cursorX = e.clientX;
cursorY = e.clientY;
c.style.left=cursorX+"px";
c.style.top=cursorY+"px";
});

// Smooth cursor trail animation
function animateTrail() {
    trailX += (cursorX - trailX) * 0.15;
    trailY += (cursorY - trailY) * 0.15;
    t.style.left = (trailX - 12) + "px";
    t.style.top = (trailY - 12) + "px";
    requestAnimationFrame(animateTrail);
}
animateTrail();

/* Canvas particles */
const canvas=document.getElementById("bg");
const ctx=canvas.getContext("2d");

canvas.width=innerWidth;
canvas.height=innerHeight;

let particles=[];
for(let i=0;i<120;i++){
particles.push({
x:Math.random()*canvas.width,
y:Math.random()*canvas.height,
vx:(Math.random()-0.5)*0.5,
vy:(Math.random()-0.5)*0.5
});
}

function draw(){
ctx.clearRect(0,0,canvas.width,canvas.height);
particles.forEach((p, index)=>{
p.x+=p.vx;
p.y+=p.vy;

// Wrap around edges
if(p.x < 0) p.x = canvas.width;
if(p.x > canvas.width) p.x = 0;
if(p.y < 0) p.y = canvas.height;
if(p.y > canvas.height) p.y = 0;

// Draw particle glow
ctx.fillStyle="rgba(0,242,255,0.3)";
ctx.beginPath();
ctx.arc(p.x, p.y, 3, 0, Math.PI * 2);
ctx.fill();

// Draw particle core
ctx.fillStyle="#00f2ff";
ctx.fillRect(p.x-1,p.y-1,2,2);

// Draw connection lines
particles.slice(index).forEach(other => {
    const dx = p.x - other.x;
    const dy = p.y - other.y;
    const dist = Math.sqrt(dx*dx + dy*dy);
    if(dist < 100) {
        ctx.strokeStyle = `rgba(0,242,255, ${0.15 - dist/100 * 0.15})`;
        ctx.lineWidth = 0.5;
        ctx.beginPath();
        ctx.moveTo(p.x, p.y);
        ctx.lineTo(other.x, other.y);
        ctx.stroke();
    }
});

});
requestAnimationFrame(draw);
}
draw();

window.addEventListener('resize', () => {
    canvas.width = innerWidth;
    canvas.height = innerHeight;
});

/* Typewriter */
const words=["Full Stack Developer","UI/UX Designer","Creative Coder"];
let i=0,j=0;

function type(){
if(i<words.length){
if(j<words[i].length){
document.getElementById("type").textContent+=words[i][j];
j++;
setTimeout(type,80);
}else{
setTimeout(()=>{
document.getElementById("type").textContent="";
j=0;i++;
type();
},1500);
}
}else{i=0;type();}
}
type();

/* Scroll reveal */
const obs=new IntersectionObserver(entries=>{
entries.forEach(e=>{
if(e.isIntersecting) e.target.classList.add("active");
});
}, { threshold: 0.1 });
document.querySelectorAll(".reveal").forEach(el=>obs.observe(el));
  
document.getElementById("year").textContent=new Date().getFullYear();

// Active Navigation Link
const sections = document.querySelectorAll('section, header');
const navLinks = document.querySelectorAll('.nav-links a');

window.addEventListener('scroll', () => {
    let current = '';
    sections.forEach(section => {
        const sectionTop = section.offsetTop;
        if(scrollY >= sectionTop - 150) {
            current = section.getAttribute('id');
        }
    });

    navLinks.forEach(link => {
        link.classList.remove('active');
        if(link.getAttribute('href') === '#'+current) {
            link.classList.add('active');
        }
    });
});

// Form handler
document.querySelector('.contact').addEventListener('submit', e => {
    e.preventDefault();
    const btn = document.querySelector('.contact button');
    btn.textContent = "sending...";
    
    setTimeout(() => {
        btn.textContent = "MESSAGE_SENT_OK";
        btn.style.background = 'var(--primary)';
        btn.style.color = '#000';
        
        setTimeout(() => {
            btn.textContent = "execute.send()";
            btn.style.background = 'none';
            btn.style.color = 'var(--primary)';
            e.target.reset();
        }, 2000);
    }, 1000);
});