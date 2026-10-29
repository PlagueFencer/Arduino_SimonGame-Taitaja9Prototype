# Simon Game Taitaja9 prototype project

## Code's explanation in visual Scratch-blocs diagrams for people that may not understand written code:
### Suomeksi
<img width="3400" height="2960" alt="1 · Yleiskatsaus" src="https://github.com/user-attachments/assets/b4fada7b-a472-4ab6-9e47-a366419b186d" />

<img width="3400" height="2960" alt="2 · Itse peli" src="https://github.com/user-attachments/assets/6e4ac404-3b1f-452f-891e-65bd38078a76" />

<img width="3400" height="2840" alt="3 · Näyttäminen ja kuuntelu" src="https://github.com/user-attachments/assets/dddde15c-75b3-4112-8d6f-000592875d89" />

<img width="3400" height="3000" alt="4 · Valot ja äänet" src="https://github.com/user-attachments/assets/b2fca366-62bb-4d37-8968-9f034e72c12c" />

### In English

<img width="3400" height="2800" alt="1 · Overview" src="https://github.com/user-attachments/assets/671b09d6-08ae-43f5-8dbb-1454e2684b1f" />

<img width="3400" height="2840" alt="2 · The game" src="https://github.com/user-attachments/assets/c7e07f66-3545-4d15-a86a-cf9cf4b9ee79" />

<img width="3400" height="2680" alt="3 · Showing   listening" src="https://github.com/user-attachments/assets/dd5ed141-99ff-49c5-a844-06a1c46d632b" />

<img width="3400" height="2880" alt="4 · Lights   sounds" src="https://github.com/user-attachments/assets/4b0ff2ea-01d0-4f0b-aea6-8125ff131ec9" />

(I used Claude to generate those diagrams)

## Web panel (score + leaderboard)

The game shows the current round, score and a top-10 leaderboard on a web page. The leaderboard is saved on the ESP32, so it survives power-offs.

**Scoring:** +50 points for every correct color, +100 for each completed round starting from round 3.

Two ways to open it (both work at the same time):

- **Wi-Fi hotspot:** connect a phone or laptop to the `SimonGame` Wi-Fi (password `simon1234`) and open http://192.168.4.1
- **USB:** plug the ESP32 into the PC, open `panel.html` in Chrome or Edge, click **Connect USB** and choose the board's port. Close the Arduino Serial Monitor first, because only one program can use the port.

Files:

- `web_panel.h`: hotspot, web server, scoring and the saved leaderboard
- `panel.html`: the panel page. After editing it, run `update_panel.ps1` to refresh `panel_html.h`, then upload again.
