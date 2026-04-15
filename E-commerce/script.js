let cart = [];
let notifTimeout;

function normalizeName(name) {
    return String(name || "").toLowerCase().trim();
}

function findMatchingCardByProductName(name) {
    const target = normalizeName(name);
    const cards = document.querySelectorAll(".card");
    for (const card of cards) {
        const title = card.querySelector("h3");
        if (!title) continue;
        const titleText = normalizeName(title.textContent);
        if (titleText.includes(target) || target.includes(titleText.split(" ")[0])) {
            return card;
        }
    }
    return null;
}

function showNotif(message, type = "success") {
    const notif = document.getElementById("notif");
    if (!notif) return;

    notif.textContent = message;
    notif.classList.add("show");
    notif.style.background = type === "error" ? "#dc2626" : "#0f766e";

    clearTimeout(notifTimeout);
    notifTimeout = setTimeout(() => {
        notif.classList.remove("show");
    }, 1800);
}

function toggleCart() {
    const sidebar = document.getElementById("cartSidebar");
    const backdrop = document.getElementById("backdrop");
    const cartBtn = document.querySelector(".cart-btn");
    if (!sidebar || !backdrop) return;

    const isOpen = sidebar.classList.toggle("active");
    backdrop.classList.toggle("active", isOpen);
    sidebar.setAttribute("aria-hidden", String(!isOpen));
    if (cartBtn) {
        cartBtn.setAttribute("aria-expanded", String(isOpen));
    }
}

function addToCart(name, price) {
    const matchedCard = findMatchingCardByProductName(name);
    const addButton = matchedCard ? matchedCard.querySelector("button") : null;
    const originalButtonText = addButton ? addButton.textContent : "";

    if (addButton) {
        addButton.classList.add("is-loading");
        addButton.innerHTML = `<span class="btn-text"><span class="btn-loader"></span>Adding...</span>`;
    }

    const completeAdd = () => {
        cart.push({ name, price });
        updateCart();
        showNotif("Produit ajoute: " + name);

        if (matchedCard) {
            matchedCard.classList.add("loading", "is-added");
            setTimeout(() => matchedCard.classList.remove("loading", "is-added"), 950);
        }

        if (addButton) {
            addButton.classList.remove("is-loading");
            addButton.textContent = originalButtonText;
        }
    };

    // Simulated short network-style delay for premium loading feedback
    setTimeout(completeAdd, 420);
}

function removeCartItemByName(name) {
    const target = normalizeName(name);
    const index = cart.findIndex((item) => normalizeName(item.name) === target);
    if (index === -1) return;
    cart.splice(index, 1);
    updateCart();
    showNotif("Item removed from cart");
}

function updateCart() {
    const list = document.getElementById("cart");
    const total = document.getElementById("total");
    const cartCount = document.getElementById("cartCount");
    const itemsTotal = document.getElementById("itemsTotal");
    const emptyState = document.getElementById("emptyCartState");

    if (!list || !total) return;

    list.innerHTML = "";
    let sum = 0;
    const grouped = new Map();

    cart.forEach((item) => {
        const key = normalizeName(item.name);
        if (!grouped.has(key)) {
            grouped.set(key, { name: item.name, price: item.price, qty: 0 });
        }
        grouped.get(key).qty += 1;
        sum += item.price;
    });

    total.innerText = sum;
    if (cartCount) {
        cartCount.innerText = cart.length;
    }
    if (itemsTotal) {
        itemsTotal.innerText = cart.length;
    }

    grouped.forEach((entry) => {
        const li = document.createElement("li");
        li.innerHTML = `
            <div class="cart-item-meta">
                <span class="cart-item-name">${entry.name}</span>
                <span class="cart-item-details">${entry.qty} x ${entry.price} TND</span>
            </div>
            <button class="remove-item-btn" onclick="removeCartItemByName('${entry.name.replace(/'/g, "\\'")}')">Remove</button>
        `;
        list.appendChild(li);
    });

    if (emptyState) {
        emptyState.classList.toggle("show", cart.length === 0);
    }
}

function sendOrder() {
    if (cart.length === 0) {
        showNotif("Votre panier est vide", "error");
        return;
    }

    fetch("http://localhost:8080/order", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(cart)
    })
    .then(res => res.text())   // 👈 مهم
    .then(data => {
        showNotif(data);       // 👈 يرجع من Java
        cart = [];             // 👈 يفرّغ السلة
        updateCart();          // 👈 يحدث الواجهة
        const sidebar = document.getElementById("cartSidebar");
        if (sidebar && sidebar.classList.contains("active")) {
            toggleCart();
        }
    })
    .catch(err => {
        console.error(err);
        showNotif("Erreur ❌", "error");
    });
}

// Initialize safe default UI state
updateCart();

document.addEventListener("click", (event) => {
    const button = event.target.closest(".card button");
    if (!button) return;
    const rect = button.getBoundingClientRect();
    button.style.setProperty("--ripple-x", `${event.clientX - rect.left}px`);
    button.style.setProperty("--ripple-y", `${event.clientY - rect.top}px`);
    button.classList.remove("ripple");
    void button.offsetWidth;
    button.classList.add("ripple");
});
