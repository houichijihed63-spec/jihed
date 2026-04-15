// LOADER
let p = 0;
let perc = document.getElementById("load-perc");
let loader = document.getElementById("loader");

let interval = setInterval(()=>{
    p += 5;
    perc.textContent = p + "%";

    if(p >= 100){
        clearInterval(interval);
        loader.style.display = "none";
    }
},50);

// TYPEWRITER
const words = ["Full Stack Developer","UI Designer","Creative Coder"];
let i = 0;
let j = 0;

function type(){
    if(j < words[i].length){
        document.getElementById("type").textContent += words[i][j];
        j++;
        setTimeout(type,80);
    }else{
        setTimeout(()=>{
            document.getElementById("type").textContent="";
            j=0;
            i=(i+1)%words.length;
            type();
        },1500);
    }
}
type();

// CONTACT FAKE SEND
document.querySelector(".contact").addEventListener("submit", e=>{
    e.preventDefault();
    alert("Message sent ✅");
});