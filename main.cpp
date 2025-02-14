
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include "Servidor_Servicios.h"
#include <thread>
#include <memory>



//Ruta del archivo de usuarios
const std::string USER_DB = "database/users.txt";




int main() {

	Servidor_Servicios serverServicios;

	sf::TcpListener listener;

	if (listener.listen(54000) != sf::Socket::Status::Done) {
		std::cerr << "Error al iniciar servidor en el puerto 54000";
		return 1;
	}
	
	std::cout << "Servidor iniciado en puerto 54000 \n";

	while (true) {
		auto client = std::make_unique<sf::TcpSocket>();
		if (listener.accept(*client) == sf::Socket::Status::Done) {

			std::cout << "Cliente conectado. Asignando hilo..." << std::endl;

			// Capturar serverServicios por referencia y client por movimiento
				std::thread([&serverServicios, client = std::move(client)]() mutable {
				serverServicios.handleClient(*client, USER_DB);

				std::cout << "Cliente desconectado. Esperando nuevas conexiones..." << std::endl;
					}).detach();
		}
		
	}
	return 0;
}