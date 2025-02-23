


#include "Servidor_Servicios.h"
#include <iostream>
#include <sstream>
#include <filesystem>






void Servidor_Servicios::iniciarServidor()
{

	//sf::IpAddress ipServidor = sf::IpAddress::Any; // Escucha en cualquier IP
	//if (UdpSocket.bind(54000, ipServidor) != sf::Socket::Status::Done) {
	//	std::cerr << "Error al iniciar Servidor_Servicios en el puerto 54000" << std::endl;
	//}


	// Enlazar al puerto 54000
	if (UdpSocket.bind(54000) != sf::Socket::Status::Done) {
		std::cerr << "Error al iniciar servidor UDP." << std::endl;
		return;
	}
	std::cout << "Servidor UDP iniciado en puerto 54000." << std::endl;


	// Cargar usuarios desde el archivo
	std::ifstream file("database/users.txt");
	std::string linea;
	while (std::getline(file, linea)) {
		size_t sep1 = linea.find(":");
		size_t sep2 = linea.find(":", sep1 + 1);
		std::string usuario = linea.substr(0, sep1);
		std::string contraseña = linea.substr(sep1 + 1, sep2 - sep1 - 1);
		int puntos = std::stoi(linea.substr(sep2 + 1));

		usuarios[usuario] = { usuario,contraseña, puntos, sf::IpAddress::Any, 0 };
	}

}

void Servidor_Servicios::procesarMensajes()
{

	while (true) {

		sf::Packet packet;
		std::optional<sf::IpAddress> sender;
		unsigned short port;

		if (UdpSocket.receive(packet, sender, port) != sf::Socket::Status::Done) {
			continue;
		}
	
		if (!sender.has_value()) {
			continue;
		}

		std::string mensaje;
		if (!(packet >> mensaje)) {
			continue;
		}

		if (mensaje.find("LOGIN:") == 0 ) {
			manejarLogin(mensaje, sender.value(), port);
		}
		else if (mensaje.find("REGISTER:") == 0) {
			manejarRegister(mensaje, sender.value(), port);
		}
		else if (mensaje.find("MATCHMAKING:") == 0) {
			manejarMatchmaking(mensaje, sender.value(), port);
		}
		else if (mensaje.find("VICTORIA:") == 0) {
			manejarVictoria(mensaje, sender.value(), port);
		}

	}


}






void Servidor_Servicios::manejarLogin(const std::string& receivedData, sf::IpAddress ip, unsigned short port)
{

	std::cout << "Datos recibidos del cliente: " << receivedData << std::endl;

	size_t pos1 = receivedData.find(":");
	size_t pos2 = receivedData.find(":", pos1 + 1);
	std::string accion = receivedData.substr(0, pos1);
	std::string user = receivedData.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string pass = receivedData.substr(pos2 + 1);

	if (usuarios.find(user) != usuarios.end() && usuarios[user].contraseña == pass) {
		usuarios[user].ip = ip;
		usuarios[user].port = port;

		sf::Packet respuesta;
		respuesta << "LOGIN_OK";
		UdpSocket.send(respuesta, ip, port);
	}
	else {
		sf::Packet respuesta;
		respuesta << "LOGIN_FAIL";
		UdpSocket.send(respuesta, ip, port);
	}




}

void Servidor_Servicios::manejarRegister(const std::string& receivedData, sf::IpAddress ip, unsigned short port)
{
	size_t pos1 = receivedData.find(":");
	size_t pos2 = receivedData.find(":", pos1 + 1);
	std::string accion = receivedData.substr(0, pos1);
	std::string user = receivedData.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string pass = receivedData.substr(pos2 + 1);

	if (usuarios.find(user) != usuarios.end()) {
		sf::Packet respuesta;
		respuesta << "REGISTER_FAIL (usuario ya existe)";
		UdpSocket.send(respuesta, ip, port);
	}
	else {
		//Registrar nuevo usuario
		usuarios[user] = { user,pass, 1000, ip, port };

		// Guardar en el archivo
		std::ofstream file("database/users.txt", std::ios::app);
		file << user << ":" << pass << ":1000\n";
		file.close();

		sf::Packet respuesta;
		respuesta << "REGISTER_OK";
		UdpSocket.send(respuesta, ip, port);
	}
}

void Servidor_Servicios::manejarMatchmaking(const std::string& data, sf::IpAddress ip, unsigned short port)
{
	std::cout << "----------Data: " << data<<std::endl;
	size_t pos1 = data.find(":"); //Posicion del primer separador
	size_t pos2 = data.find(":", pos1 + 1); // Posición del segundo separador

	std::string nombre = data.substr(pos1 + 1, pos2 - pos1 - 1); // Extraer nombre
	std::cout << "----------Nombre jugador : " << nombre << std::endl;
	
	std::string modo = data.substr(pos2 + 1); // Extraer modo (AMISTOSO/COMPETITIVO)
	std::cout << "----------MODO jugador : " << modo << std::endl;
	Jugador jugador = { nombre,usuarios[nombre].contraseña, usuarios[nombre].puntos, ip, port };
	

	if (modo == "AMISTOSO") {
		colaAmistosa.push(jugador);
	}
	else if (modo == "COMPETITIVO") {
		colaCompetitiva.push_back(jugador);
	}

	emparejarJugadores();



}

void Servidor_Servicios::manejarVictoria(const std::string& data, sf::IpAddress ip, unsigned short port)
{
	try{
	// Validar que haya dos ":" en el mensaje
	size_t pos1 = data.find(":");
	size_t pos2 = data.find(":", pos1 + 1);

	// Validar formato del mensaje
	
	if (pos1 == std::string::npos || pos2 == std::string::npos) {
		std::cerr << "Formato de victoria inválido. Mensaje recibido: " << data << std::endl;
	}
	std::string usuario = data.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string strPuntos = data.substr(pos2 + 1);
	// Validar que los puntos sean un número
	if (strPuntos.empty() || !std::all_of(strPuntos.begin(), strPuntos.end(), ::isdigit)) {
		std::cerr << "Error: Puntos no válidos. Mensaje: " << data << std::endl;
		return;
	}

	int puntos = std::stoi(strPuntos);
	// Verificar si el usuario existe
	if (usuarios.find(usuario) == usuarios.end()) {
		std::cerr << "Usuario no encontrado: " << usuario << std::endl;
		return;
	}

	// Actualizar puntos
	usuarios[usuario].puntos += puntos;

	// Actualizar base de datos


		// Guardar en archivo
	std::ofstream file("database/users.txt");
	if (!file.is_open()) {
		std::cerr << "Error al abrir users.txt para escritura." << std::endl;
		return;
	}

	for (const auto& [nombre, jugador] : usuarios) {
		file << nombre << ":" << jugador.contraseña << ":" << jugador.puntos << "\n";
	}
	file.close();

		// Imprimir ranking
		std::cout << "\n=== RANKING ACTUALIZADO ===";
		for (auto& [nombre, jugador] : usuarios) {
			std::cout << "\n" << nombre << ": " << jugador.puntos << " puntos";
		}
		std::cout << "\n==========================\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error en manejarVictoria: " << e.what() << std::endl;
	}



}

void Servidor_Servicios::emparejarJugadores()
{
	// Modo Amistoso
	while (colaAmistosa.size() >= 2) {
		Jugador jugador1 = colaAmistosa.front();
		colaAmistosa.pop();
		Jugador jugador2 = colaAmistosa.front();
		colaAmistosa.pop();
		notificarPartida(jugador1, jugador2);
	}

	// Modo Competitivo

	static int rango = 30; //Rango inicial
	static auto ultimoAumento = std::chrono::steady_clock::now();

	//Aumentar el rango cada 5 segundos
	auto ahora = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(ahora - ultimoAumento).count() >= 5) {
		rango = std::min(rango + 1, 50); // Aumentar el rango, máximo 50
		ultimoAumento = ahora;
	}
	
		// Buscar emparejamientos dentro del rango
	for (auto it = colaCompetitiva.begin(); it != colaCompetitiva.end(); ++it) {
		for (auto it2 = it + 1; it2 != colaCompetitiva.end(); ++it2) {
			if (std::abs(it->puntos - it2->puntos) <= rango) {
				// Emparejar a los jugadores
				Jugador jugador1 = *it;
				Jugador jugador2 = *it2;

				// Eliminar a los jugadores de la cola
				colaCompetitiva.erase(it2);
				colaCompetitiva.erase(it);

				// Notificar a los jugadores
				notificarPartida(jugador1, jugador2);
				return; // Salir después de emparejar
			}
		}
	}



}

void Servidor_Servicios::notificarPartida(Jugador& jugador1, Jugador& jugador2)
{

	// Dirección y puerto del Server_Gameplay
	sf::IpAddress gameplayServerIp = sf::IpAddress::LocalHost; // -- LOCALHOST
	
	//std::optional<sf::IpAddress> gameplayServerIp = sf::IpAddress::resolve("192.168.1.100");
	



	// Notificar al servidor gameplay -- LOCALHOST
	sf::Packet packet1, packet2;
	packet1 << "MATCH_FOUND:" + serverAddressGameplay.toString() + ":" + std::to_string(serverPortGameplay);
	packet2 << "MATCH_FOUND:" + serverAddressGameplay.toString() + ":" + std::to_string(serverPortGameplay);


	// Notificar al servidor gameplay
	//sf::Packet packet1, packet2;
	//packet1 << "MATCH_FOUND:" + serverAddressGameplay.value().toString() + ":" + std::to_string(serverPortGameplay);
	//packet2 << "MATCH_FOUND:" + serverAddressGameplay.value().toString() + ":" + std::to_string(serverPortGameplay);


	UdpSocket.send(packet1, jugador1.ip, jugador1.port);
	UdpSocket.send(packet2, jugador2.ip, jugador2.port);

	

	std::cout << "Partida iniciada entre " << jugador1.nombre << " y " << jugador2.nombre << std::endl;

}



