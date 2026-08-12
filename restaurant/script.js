let cart = JSON.parse(localStorage.getItem('cart')) || [];
let total = cart.reduce((sum, item) => sum + item.price, 0);

document.addEventListener('DOMContentLoaded', () => {
    updateCart();
    initTheme();
    showOnScroll();
});

function addToCart(name, price) {
    cart.push({name, price});
    total += price;
    saveCart();
    updateCart();
    showToast(`✅ ${name} ajouté au panier`);
    
    document.querySelector('.cart-icon').classList.add('bounce');
    setTimeout(() => document.querySelector('.cart-icon').classList.remove('bounce'), 400);
}

function updateCart() {
    const list = document.getElementById("cart-items");
    const count = document.getElementById("count");

    list.innerHTML = "";

    cart.forEach((item, i) => {
        list.innerHTML += `
        <li>
            <div>
                <div>${item.name}</div>
                <small>${item.price} DA</small>
            </div>
            <button onclick="removeItem(${i})" aria-label="Supprimer">❌</button>
        </li>`;
    });

    document.getElementById("total").innerText = total;
    count.innerText = cart.length;
}

function removeItem(i) {
    showToast(`❌ ${cart[i].name} supprimé`);
    total -= cart[i].price;
    cart.splice(i, 1);
    saveCart();
    updateCart();
}

function toggleCart() {
    document.getElementById("cart").classList.toggle("show");
    document.querySelector('.overlay').classList.toggle('show');
}

function sendOrder() {
    if(cart.length === 0) {
        showToast("⚠️ Votre panier est vide !");
        return;
    }
    
    const phone = "21658020069";
    let msg = `🍔 *Nouvelle Commande Délice*\n\n`;

    cart.forEach(item => {
        msg += `✅ ${item.name} - ${item.price} DA\n`;
    });

    msg += `\n💰 *Total: ${total} DA*`;
    const url = "https://wa.me/" + phone + "?text=" + encodeURIComponent(msg);
    window.open(url, '_blank');
}

function pay() {
    if(cart.length === 0) {
        showToast("⚠️ Votre panier est vide !");
        return;
    }

    showToast("✅ Paiement effectué avec succès !");
    cart = [];
    total = 0;
    saveCart();
    updateCart();
    
    setTimeout(() => toggleCart(), 1500);
}

function toggleDark() {
    document.body.classList.toggle("light");
    localStorage.setItem('theme', document.body.classList.contains('light') ? 'light' : 'dark');
}

function initTheme() {
    if(localStorage.getItem('theme') === 'light') {
        document.body.classList.add('light');
    }
}

function saveCart() {
    localStorage.setItem('cart', JSON.stringify(cart));
}

function showToast(message) {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.classList.add('show');
    
    setTimeout(() => toast.classList.remove('show'), 2500);
}

// Scroll Animation with debounce
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        clearTimeout(timeout);
        timeout = setTimeout(() => func(...args), wait);
    };
}

const showOnScroll = debounce(() => {
    const elements = document.querySelectorAll('.fade-in');
    elements.forEach(el => {
        if (el.getBoundingClientRect().top < window.innerHeight - 100) {
            el.classList.add('show');
        }
    });
}, 10);

window.addEventListener('scroll', showOnScroll);