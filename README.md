# PowerPing

PowerPing is a lightweight Windows utility written in C that intelligently monitors your laptop's battery charge level and automatically sends an email notification when the battery reaches a configurable threshold. It uses the Windows Power API to retrieve battery information, **libcurl** for secure SMTP email delivery, and **cJSON** for configuration management.

---

## Features

- Intelligent battery monitoring using the Windows Power API
- Adaptive polling intervals for minimal CPU usage
- Automatic email notifications via SMTP
- JSON-based configuration
- Lightweight native C application
- No GUI required
- Low memory and CPU footprint
- Easy to compile and run

---

## How It Works

PowerPing continuously monitors your laptop's battery percentage.

Instead of checking the battery every few seconds, it intelligently adjusts the polling interval based on the current battery level, reducing unnecessary CPU wake-ups while still providing timely notifications.

| Battery Level          | Check Interval     |
| ---------------------- | ------------------ |
| 0–39%                  | 11 Minutes         |
| 40–59%                 | 8 Minutes          |
| 60–79%                 | 5 Minutes          |
| 80–92%                 | 2 Minutes          |
| ≥ Configured Threshold | Email Notification |

---

## Project Structure

```text
PowerPing/
│
├── overcharge_notifier.c
├── config.json
├── README.md
└── LICENSE
```

---

## Requirements

- Windows 10 / Windows 11
- GCC (MinGW)
- libcurl
- cJSON

---

## Configuration

Create a `config.json` file in the application directory.

```json
{
  "sender_email": "your-email@gmail.com",
  "sender_password": "your-google-app-password",
  "receiver_email": "receiver@gmail.com",
}
```

### Configuration Parameters

| Key               | Description                                       |
| ----------------- | ------------------------------------------------- |
| `sender_email`    | Email account used to send notifications          |
| `sender_password` | Google App Password                               |
| `receiver_email`  | Email address that receives notifications         |

---

## Gmail Configuration

Google requires an **App Password** for SMTP authentication.

1. Enable **Two-Step Verification** on your Google account.
2. Generate a **Google App Password**.
3. Use the generated password in `config.json`.

---

## Build

Compile using GCC:

```bash
gcc power_ping.c -o PowerPing.exe -lcurl -lcjson
```

---

## Run

```bash
PowerPing.exe
```

The application will continue monitoring your battery in the background and automatically send an email once the configured threshold is reached.

---

## Example Notification

**Subject**

```text
Battery reached 95%
```

**Body**

```text
Your laptop battery has reached 95%.

Please unplug the charger.
```

---

## Libraries Used

- libcurl
- cJSON
- Windows Power Management API (`GetSystemPowerStatus`)

---

## Roadmap

- [ ] Windows toast notifications
- [ ] System tray integration
- [ ] Custom notification thresholds
- [ ] Multiple notification providers
  - [ ] Email
  - [ ] Telegram
  - [ ] Discord
  - [ ] Slack
- [ ] Battery health analytics
- [ ] Logging support
- [ ] Automatic startup with Windows
- [ ] Windows installer

---

## Contributing

Contributions, feature requests, and bug reports are welcome.

If you'd like to contribute, feel free to fork the repository and submit a pull request.

---

## License

This project is licensed under the **GNU General Public License v3.0**.

See the `LICENSE` file for more information.

---

## Author

**Rohit Viswakarma Pidishetti**

GitHub: https://github.com/rohitpidishetty

---

## Support

If you find PowerPing useful, consider giving this repository a **⭐** on GitHub!
