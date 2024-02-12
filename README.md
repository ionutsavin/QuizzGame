
# QuizzGame

## 🚀 Project Overview

Welcome to QuizzGame, a personal project developed to create an engaging, multithreaded quiz server. This high-energy server application hosts real-time quiz competitions for multiple clients, showcasing both knowledge and quick decision-making in a fun, competitive environment. Efficiently managing multiple client connections simultaneously, QuizzGame ensures an uninterrupted and thrilling quiz experience for all participants.

### Features

- **Real-Time Interaction**: Players compete simultaneously in a fast-paced quiz.
- **Time-Bound Challenges**: Each question is timed, adding excitement and urgency to the quiz.
- **Fair and Controlled Participation**: The server manages a fixed registration window, ensuring equitable participation.
- **Dynamic Question Bank**: Questions and answers stored in XML format, providing ease of updates and modifications.
- **Score Tracking with SQLite**: Real-time tracking of participant scores, keeping the competition lively.
- **Graceful Error Handling**: Smooth handling of client dropouts to maintain continuous gameplay.

### Technologies

- **TCP/IP Sockets**: For robust client-server communication.
- **XML Parsing with libxml2**: Featuring libxml/parser.h and libxml/tree.h for efficient XML data management.
- **SQLite Database**: For effective real-time score tracking and participant data management.
- **C Programming Language**: The backbone of server and client application development.

### Getting Started

1. **Clone the Repository**

   ```bash
   git clone https://github.com/ionutsavin/QuizzGame.git
   ```

2. **Navigate to the Project Directory**

   ```bash
   cd QuizzGame
   ```

3. **Build the Server and Client**

   - The Makefile simplifies compilation.

   ```bash
   make
   ```

4. **Launch the Server**

   ```bash
   ./server
   ```

5. **Connect Clients**

   ```bash
   ./client
   ```

6. **Manage Questions**

   - Questions can be added or updated in questions.xml.
