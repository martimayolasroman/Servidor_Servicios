


#include "Servidor_Servicios.h"
#include <iostream>

void Servidor_Servicios::handleClient(sf::TcpSocket& socket, const std::string& USER_DB) {
	sf::Packet packet;
	std::string receivedData;
	

	// Recibir el paquete del cliente
	sf::Socket::Status status = socket.receive(packet);
	if (status != sf::Socket::Status::Done) {
		std::cerr << "Error recibiendo datos del cliente.  "<< std::endl;
		return;
	}

	// Verificar que el paquete no está vacío
		if (packet.getDataSize() == 0) {
			std::cerr << "Error: Paquete recibido está vacío." << std::endl;
			return;
		}

		// Extraer los datos del paquete
		if (!(packet >> receivedData)) {
			std::cerr << "Error al leer los datos del paquete recibido." << std::endl;
			return;
		}

		std::cout << "Datos recibidos del cliente: " << receivedData << std::endl;

		// Procesar la acción (LOGIN o REGISTER)
		sf::Packet response;
		std::string responseMessage;
		std::string action, user, pass;

		// Separar los datos enviados (Formato esperado: "REGISTER:usuario:contraseña")
		size_t pos1 = receivedData.find(":");
		size_t pos2 = receivedData.find(":", pos1 + 1);

		if(pos1 == std::string::npos || pos2 == std::string::npos) {
			std::cerr << "Error: Formato de datos incorrecto." << std::endl;
			responseMessage = "ERROR_FORMAT";
		}
		else {

			action = receivedData.substr(0, pos1);
			user = receivedData.substr(pos1 + 1, pos2 - pos1 - 1);
			pass = receivedData.substr(pos2 + 1);

			if (action == "REGISTER") {
				//Registrar usuario
				std::ofstream file(USER_DB, std::ios::app);
				if (file) {
					file << user << ":" << pass << "\n";
					//file.close();
					response << "REGISTER_OK";
					std::cout << "Usuario registrado correctamente: " << user << std::endl;
				}
				else {

					response << "REGISTER_FAIL";
					std::cerr << "Error al registrar usuario." << std::endl;


				}

			}
			else if (action == "LOGIN") {
				//Verificar credenciales
				std::ifstream file(USER_DB);
				std::string line;
				bool found = false;
				while (std::getline(file, line)) {
					if (line == user + ":" + pass) {
						found = true;
						break;
					}
				}

				responseMessage = (found ? "LOGIN_OK" : "LOGIN_FAIL");
			}
			else{
				responseMessage = "ERROR_ACTION";
			}

		}



		response << responseMessage;

		// Enviar respuesta
		if (socket.send(response) != sf::Socket::Status::Done) {
			std::cerr << "Error enviando respuesta al cliente." << std::endl;
		}
		else {
			std::cout << "Respuesta enviada al cliente: " << responseMessage << std::endl;
		}
	
}


