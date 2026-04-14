let cart = [];

function addToCart(name, price) {
    alert("Produit ajouté: " + name);

    cart.push({ name, price });
    updateCart();
}

function updateCart() {
    let list = document.getElementById("cart");
    let total = document.getElementById("total");

    list.innerHTML = "";
    let sum = 0;

    cart.forEach(item => {
        list.innerHTML += `<li>${item.name} - ${item.price} DA</li>`;
        sum += item.price;
    });

    total.innerText = sum;
}

function sendOrder() {
    fetch("http://localhost:8080/order", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(cart)
    })
    .then(res => res.text())   // 👈 مهم
    .then(data => {
        alert(data);           // 👈 يرجع من Java
        cart = [];             // 👈 يفرّغ السلة
        updateCart();          // 👈 يحدث الواجهة
    })
    .catch(err => {
        console.error(err);
        alert("Erreur ❌");
    });
}
