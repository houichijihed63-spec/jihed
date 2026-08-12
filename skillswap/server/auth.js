const jwt = require('jsonwebtoken');
const bcrypt = require('bcryptjs');
const db = require('./database');

const JWT_SECRET = "SKILLSWAP_AI_SECRET_2026_PRODUCTION";

// انشاء حساب جديد
exports.register = (req, res) => {
  const { name, email, password, skills, bio } = req.body;
  const hashed = bcrypt.hashSync(password, 10);

  db.run(`INSERT INTO users (name, email, password, skills, bio) VALUES (?, ?, ?, ?, ?)`,
    [name, email, hashed, JSON.stringify(skills), bio],
    function(err) {
      if(err) return res.status(400).json({error: "البريد الالكتروني موجود مسبقاً"});
      
      const token = jwt.sign({id: this.lastID}, JWT_SECRET, {expiresIn: '30d'});
      res.json({token, user: {id: this.lastID, name, email, skills, bio, rating:0}});
    }
  )
}

// تسجيل الدخول
exports.login = (req, res) => {
  const { email, password } = req.body;

  db.get(`SELECT * FROM users WHERE email = ?`, [email], (err, user) => {
    if(!user || !bcrypt.compareSync(password, user.password)) 
      return res.status(401).json({error: "بيانات الدخول خاطئة"});

    user.skills = JSON.parse(user.skills);
    delete user.password;

    const token = jwt.sign({id: user.id}, JWT_SECRET, {expiresIn: '30d'});
    res.json({token, user});
  })
}

// وسيط التحقق من الهوية
exports.authMiddleware = (req, res, next) => {
  const token = req.headers.authorization?.split(' ')[1];
  if(!token) return res.status(401).json({error: "غير مصرح"});

  try {
    req.user = jwt.verify(token, JWT_SECRET);
    next();
  } catch {
    res.status(401).json({error: "جلسة منتهية"});
  }
}