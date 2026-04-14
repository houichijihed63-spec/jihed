const express = require('express');
const cors = require('cors');
const db = require('./database');
const auth = require('./auth');

const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('client'));

// Routes Auth
app.post('/api/register', auth.register);
app.post('/api/login', auth.login);

// ✨ نظام الماتشينج الذكي AI
app.get('/api/matches', auth.authMiddleware, (req, res) => {
  db.all(`SELECT * FROM users WHERE id != ?`, [req.user.id], (err, users) => {
    db.get(`SELECT skills FROM users WHERE id = ?`, [req.user.id], (e, current) => {
      const mySkills = JSON.parse(current.skills);

      // خوارزمية حساب نسبة التطابق
      users.forEach(user => {
        const userSkills = JSON.parse(user.skills);
        const common = mySkills.filter(s => userSkills.includes(s)).length;
        user.match_score = Math.round((common / mySkills.length) * 100);
        delete user.password;
      });

      // ترتيب حسب اعلى نسبة مطابقة
      users.sort((a,b) => b.match_score - a.match_score);
      res.json(users);
    })
  })
})

// باقي الـ API
app.get('/api/users/:id', (req, res) => {
  db.get(`SELECT * FROM users WHERE id = ?`, [req.params.id], (err, user) => {
    user.skills = JSON.parse(user.skills);
    delete user.password;
    res.json(user);
  })
})

app.get('/api/chat/:user', auth.authMiddleware, (req, res) => {
  db.all(`SELECT * FROM messages WHERE 
    (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)
    ORDER BY created_at ASC`,
    [req.user.id, req.params.user, req.params.user, req.user.id],
    (err, messages) => res.json(messages)
  )
})

app.post('/api/chat/:user', auth.authMiddleware, (req, res) => {
  db.run(`INSERT INTO messages (sender, receiver, content) VALUES (?, ?, ?)`,
    [req.user.id, req.params.user, req.body.content],
    () => res.json({success:true})
  )
})

app.listen(3000, () => console.log("✅ SkillSwap AI يعمل على البورت 3000"));