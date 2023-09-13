# Tor Network - Ziv and Uriyah

![image info](https://drawcartoonstyle.com/wp-content/uploads/2022/09/9-add-kawaii-blush-spots-to-the-onion-768x768.jpg)

The Simple Tor Network POC is an experimental project aimed at understanding the fundamentals of building a basic anonymity network. Tor, short for "The Onion Router," is a well-known example of such a network. The POC currently utilizes Python and RSA encryption for its operations.

**Getting Started**

To get started with the Simple Tor Network POC, follow these steps:

## **Clone the Repository:**

**bash**

`git clone https://gitlab.com/ziv_sion_advanced_programming_2023/tor-network-ziv-and-uriyah.git`

**Install Dependencies:**

Make sure you have Python 3.x installed on your system. Additionally, you may need to install any Python dependencies specified in the project's requirements.txt file. You can use pip for this purpose:

`pip install -r requirements.txt`


## **Run the POC:**

You can run the POC using the provided Python scripts. Detailed usage instructions can be found in the next section.

Usage
Currently, the POC is a command-line tool. To run the POC, follow these steps:

- **Start the parent server:**

`python tor_server.py`

- **Start a Tor Node:**

`python tor_node.py`

This will initialize a Tor node on your machine. You can start multiple nodes on different ports to create a network.

- **Start a Client:**

`python client.py`
The client will establish a connection to the Tor network and allow you to send messages through the network.

## **Explore the Code:**

To gain a deeper understanding of how the POC works, explore the Python source code in the repository.

Contributing
We welcome contributions from the open-source community! If you'd like to contribute to the development of this project, please follow these guidelines:

Fork the repository.
Create a new branch for your feature or bug fix.
Make your changes and ensure they are well-documented.
Test your changes thoroughly.
Submit a pull request.
Future Development
Our plans for future development of the Simple Tor Network POC include:

Rewriting the codebase in C++ for improved performance and security.
Replacing RSA encryption with AES for enhanced security.
Adding a graphical user interface (GUI) to simplify user interaction.
Stay tuned for updates and consider contributing to these exciting developments!

## **License**

This project is licensed under the MIT License - see the LICENSE file for details
