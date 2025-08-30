# encrypted-chat (Learning Project)

📚 About This Project  
A small text chat in C experimenting with XOR-based message obfuscation and sockets.  

The XOR part works — messages can be encoded and decoded correctly.  
Where I struggled was with **threading and stdin handling**. It became frustrating, messy, and mentally exhausting.  
I’m leaving it here as a record of what worked, what didn’t, and where I hit a wall.

---

🛠️ What Works  
- XOR encryption/decryption ✅  
- Basic client/server socket communication ✅  

---

⚠️ Where I Struggled  
- Threading + stdin ❌  
- Input handling and synchronization caused unpredictable behavior  
- Debugging this part was draining and hurting me mentally, since the main goal of the project was the XOR encryption part

---

💡 What I Learned  
- Implementing XOR-based message obfuscation  
- Setting up simple socket communication  
- Concurrency in C is tricky and very sensitive to handle

---

🔒 Final Notes  
This isn’t a polished chat app — it’s a snapshot of my learning journey.  
It shows what worked, what failed, and why I chose to step away.
