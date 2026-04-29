/**
 * jihed.exe - Advanced Portfolio Controller v2.0
 * Principal UI Engineer Edition
 */

class PortfolioController {
  constructor() {
    this.isMobile = window.innerWidth <= 768;
    this.scrollY = 0;
    this.init();
  }

  init() {
    this.cacheElements();
    this.bindEvents();
    this.initPreloader();
    this.initNavigation();
    this.initScrollEffects();
    this.initAnimations();
    this.initTerminal();
    this.initFormHandler();
    this.initParticles();
    this.initSmoothScroll();
  }

  cacheElements() {
    this.elements = {
      loader: document.getElementById('loader'),
      loaderProgress: document.getElementById('loader-progress'),
      header: document.getElementById('header'),
      nav: document.getElementById('nav'),
      menuToggle: document.getElementById('menu-toggle'),
      terminalBody: document.getElementById('terminal-body'),
      typingLine: document.getElementById('typing-line'),
      contactForm: document.getElementById('contact-form')
    };
  }

  bindEvents() {
    window.addEventListener('resize', this.handleResize.bind(this), { passive: true });
    window.addEventListener('scroll', this.handleScroll.bind(this), { passive: true });
  }

  // 🎯 Preloader
  initPreloader() {
    let progress = 0;
    const increment = () => {
      progress += Math.random() * 12;
      if (progress > 95) progress = 95;
      
      this.elements.loaderProgress.style.width = `${progress}%`;
      
      if (progress < 95) {
        requestAnimationFrame(increment);
      } else {
        setTimeout(() => {
          this.elements.loaderProgress.style.width = '100%';
          setTimeout(() => {
            this.elements.loader.classList.add('hidden');
            document.body.classList.add('loaded');
          }, 600);
        }, 400);
      }
    };
    increment();
  }

  // 📱 Navigation
  initNavigation() {
    this.elements.menuToggle.addEventListener('click', this.toggleMobileMenu.bind(this));
    
    document.querySelectorAll('.nav-link').forEach(link => {
      link.addEventListener('click', this.handleNavClick.bind(this));
    });

    // Keyboard support
    document.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') {
        this.closeMobileMenu();
      }
    });
  }

  toggleMobileMenu() {
    document.body.classList.toggle('nav-open');
    this.elements.menuToggle.classList.toggle('active');
  }

  closeMobileMenu() {
    document.body.classList.remove('nav-open');
    this.elements.menuToggle.classList.remove('active');
  }

  handleNavClick(e) {
    e.preventDefault();
    const section = e.target.getAttribute('data-section');
    this.scrollToSection(section);
    this.closeMobileMenu();
    this.updateActiveNav(e.target);
  }

  updateActiveNav(activeLink) {
    document.querySelectorAll('.nav-link').forEach(link => link.classList.remove('active'));
    activeLink.classList.add('active');
  }

  scrollToSection(sectionId) {
    const section = document.getElementById(sectionId);
    section?.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }

  // 🎨 Scroll Effects
  initScrollEffects() {
    // Header scroll effect
    const heroObserver = new IntersectionObserver(([entry]) => {
      this.elements.header.classList.toggle('scrolled', !entry.isIntersecting);
    }, { 
      threshold: 0, 
      rootMargin: '-80px 0px 0px 0px' 
    });

    heroObserver.observe(document.getElementById('home'));

    // Active nav link on scroll
    this.sectionObserver = new IntersectionObserver((entries) => {
      entries.forEach(entry => {
        if (entry.isIntersecting) {
          const sectionId = entry.target.getAttribute('id');
          document.querySelectorAll('.nav-link').forEach(link => {
            link.classList.toggle('active', link.getAttribute('data-section') === sectionId);
          });
        }
      });
    }, { 
      threshold: 0.3,
      rootMargin: '-20% 0px -20% 0px'
    });

    document.querySelectorAll('.section').forEach(section => {
      this.sectionObserver.observe(section);
    });
  }

  handleScroll() {
    this.scrollY = window.pageYOffset;
  }

  // ✨ Scroll Animations
  initAnimations() {
    this.scrollObserver = new IntersectionObserver((entries) => {
      entries.forEach((entry, index) => {
        if (entry.isIntersecting) {
          const delay = index * 150;
          setTimeout(() => {
            entry.target.classList.add('animate-in');
          }, delay);
        }
      });
    }, {
      threshold: 0.1,
      rootMargin: '0px 0px -10% 0px'
    });

    document.querySelectorAll('[data-aos]').forEach(el => {
      this.scrollObserver.observe(el);
    });
  }

  // 💻 Terminal Animation
  initTerminal() {
    const commands = [
      { input: 'whoami', output: 'jihed.exe - Principal UI Engineer' },
      { input: 'portfolio --status', output: 'Status: LIVE 🚀 99.9% uptime' },
      { input: 'npm run skills', output: 'React | Node.js | TypeScript | Docker' },
      { input: 'node career.js', output: 'Career trajectory: ASCENDING 📈' }
    ];

    let commandIndex = 0;
    
    const typeWriter = (text, element, callback, isInput = false) => {
      let i = 0;
      element.textContent = isInput ? '$ ' : '';
      element.classList.add('typing');

      const typeChar = () => {
        if (i < text.length) {
          element.textContent += text.charAt(i);
          i++;
          setTimeout(typeChar, 60 + Math.random() * 40);
        } else {
          element.classList.remove('typing');
          setTimeout(callback, 1200);
        }
      };
      typeChar();
    };

    const addCommand = () => {
      if (commandIndex < commands.length) {
        const command = commands[commandIndex];
        
        // Add input line
        const inputLine = document.createElement('div');
        inputLine.className = 'terminal-line';
        const inputSpan = document.createElement('span');
        inputSpan.className = 'prompt';
        inputSpan.textContent = '$';
        inputLine.appendChild(inputSpan);
        this.elements.terminalBody.appendChild(inputLine);

        typeWriter(command.input, inputLine.lastChild.nextSibling, () => {
          // Add output line
          const outputLine = document.createElement('div');
          outputLine.className = 'terminal-output gradient-text';
          outputLine.textContent = command.output;
          this.elements.terminalBody.appendChild(outputLine);
          
          this.elements.terminalBody.scrollTop = this.elements.terminalBody.scrollHeight;
          
          commandIndex++;
          setTimeout(addCommand, 800);
        }, true);
      }
    };

    setTimeout(addCommand, 1500);
  }

  // 📝 Form Handler
  initFormHandler() {
    this.elements.contactForm.addEventListener('submit', this.handleFormSubmit.bind(this));

    // Input effects
    this.elements.contactForm.querySelectorAll('input, textarea').forEach(input => {
      input.addEventListener('focus', () => input.closest('.form-group').classList.add('focused'));
      input.addEventListener('blur', () => input.closest('.form-group').classList.remove('focused'));
    });
  }

  async handleFormSubmit(e) {
    e.preventDefault();
    const formData = new FormData(e.target);
    const submitBtn = e.target.querySelector('button[type="submit"]');
    const originalText = submitBtn.innerHTML;

    // Loading state
    submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Sending...';
    submitBtn.disabled = true;

    try {
      // Simulate API call
      await new Promise(resolve => setTimeout(resolve, 2000));
      
      // Success
      submitBtn.innerHTML = '<i class="fas fa-check"></i> Sent!';
      e.target.reset();
      this.showNotification('Message sent successfully! 🚀', 'success');
      
      setTimeout(() => {
        submitBtn.innerHTML = originalText;
        submitBtn.disabled = false;
      }, 2500);
      
    } catch (error) {
      this.showNotification('Failed to send message. Please try again.', 'error');
      submitBtn.innerHTML = originalText;
      submitBtn.disabled = false;
    }
  }

  // 🔔 Notifications
  showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification notification--${type}`;
    notification.innerHTML = `
      <i class="fas ${type === 'success' ? 'fa-check-circle' : 'fa-exclamation-circle'}"></i>
      ${message}
      <button class="notification-close">&times;</button>
    `;
    
    document.body.appendChild(notification);
    
    setTimeout(() => notification.classList.add('show'), 100);
    
    notification.querySelector('.notification-close').addEventListener('click', () => {
      notification.classList.remove('show');
      setTimeout(() => notification.remove(), 300);
    });
    
    setTimeout(() => {
      notification.classList.remove('show');
      setTimeout(() => notification.remove(), 300);
    }, 5000);
  }

  // 🎪 Particles
  initParticles() {
    const canvas = document.createElement('canvas');
    canvas.className = 'particles-canvas';
    document.body.appendChild(canvas);
    
    const ctx = canvas.getContext('2d');
    let particles = [];
    let animationId;
    let mouseX = 0;
    let mouseY = 0;

    const resizeCanvas = () => {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
    };

    class Particle {
      constructor() {
        this.reset();
      }
      
      reset() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.size = Math.random() * 2.5 + 0.5;
        this.speedX = (Math.random() - 0.5) * 0.8;
        this.speedY = (Math.random() - 0.5) * 0.8;
        this.opacity = Math.random() * 0.4 + 0.1;
        this.hue = 240 + Math.random() * 60;
      }
      
      update() {
        // Mouse attraction
        const dx = mouseX - this.x;
        const dy = mouseY - this.y;
        const distance = Math.sqrt(dx * dx + dy * dy);
        
        if (distance < 100) {
          this.speedX += dx * 0.01;
          this.speedY += dy * 0.01;
        }
        
        this.x += this.speedX;
        this.y += this.speedY;
        
        if (this.x < 0 || this.x > canvas.width) this.speedX *= -1;
        if (this.y < 0 || this.y > canvas.height) this.speedY *= -1;
        
        this.opacity += (Math.random() - 0.5) * 0.015;
        this.opacity = Math.max(0.05, Math.min(0.6, this.opacity));
      }
      
      draw() {
        ctx.save();
        ctx.globalAlpha = this.opacity;
        ctx.fillStyle = `hsl(${this.hue}, 70%, ${40 + this.opacity * 40}%)`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
        ctx.shadowBlur = 10;
        ctx.shadowColor = `hsl(${this.hue}, 70%, 50%)`;
        ctx.fill();
        ctx.restore();
      }
    }

    const animate = () => {
      ctx.fillStyle = 'rgba(12, 12, 12, 0.1)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      
      if (particles.length < 120) {
        particles.push(new Particle());
      }
      
      particles.forEach((particle, index) => {
        particle.update();
        particle.draw();
        
        // Remove old particles
        if (particle.opacity < 0.05 && Math.random() > 0.98) {
          particles.splice(index, 1);
        }
      });
      
      animationId = requestAnimationFrame(animate);
    };

    // Mouse tracking
    document.addEventListener('mousemove', (e) => {
      mouseX = e.clientX;
      mouseY = e.clientY;
    });

    window.addEventListener('resize', resizeCanvas);
    resizeCanvas();
    animate();

    this.particlesCleanup = () => cancelAnimationFrame(animationId);
  }

  // 🛠 Smooth Scroll
  initSmoothScroll() {
    document.querySelectorAll('[data-scroll]').forEach(link => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const href = link.getAttribute('href');
        const target = document.querySelector(href);
        target?.scrollIntoView({ behavior: 'smooth', block: 'start' });
      });
    });
  }

  handleResize() {
    this.isMobile = window.innerWidth <= 768;
    this.closeMobileMenu();
  }

  // 🧹 Cleanup
  destroy() {
    if (this.sectionObserver) this.sectionObserver.disconnect();
    if (this.scrollObserver) this.scrollObserver.disconnect();
    if (this.particlesCleanup) this.particlesCleanup();
  }
}

// 🌟 Initialize
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => new PortfolioController());
} else {
  new PortfolioController();
}

// 🔔 Notification Styles (Inline)
const notificationStyles = `
.notification {
  position: fixed;
  top: 100px;
  right: 20px;
  background: rgba(255,255,255,0.95);
  backdrop-filter: blur(20px);
  border: 1px solid rgba(255,255,255,0.2);
  border-radius: 12px;
  padding: 1.25rem 1.5rem;
  box-shadow: var(--shadow);
  transform: translateX(400px);
  transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
  z-index: 10000;
  max-width: 400px;
  display: flex;
  align-items: center;
  gap: 0.75rem;
  font-weight: 500;
}

.notification--success { border-left: 4px solid #10b981; }
.notification--error { border-left: 4px solid #ef4444; }

.notification.show {
  transform: translateX(0);
}

.notification-close {
  margin-left: auto;
  background: none;
  border: none;
  font-size: 1.5rem;
  cursor: pointer;
  color: var(--text-muted);
  padding: 0;
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  transition: all 0.2s ease;
}

.notification-close:hover {
  background: rgba(0,0,0,0.1);
  color: #ef4444;
}

.particles-canvas {
  position: fixed;
  top: 0;
  left: 0;
  pointer-events: none;
  z-index: 1;
}
`;

const styleSheet = document.createElement('style');
styleSheet.textContent = notificationStyles;
document.head.appendChild(styleSheet);