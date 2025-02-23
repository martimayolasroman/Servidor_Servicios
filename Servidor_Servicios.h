#pragma once

#include <SFML/Network.hpp>
#include <fstream>
#include <map>
#include <queue>

#include <unordered_map>
#include <thread>
#include <optional>
#include <chrono>



struct Jugador {
	std::string nombre;
	std::string contraseña;
	int puntos = 1000;//puntos inicial
	sf::IpAddress ip;
	unsigned short port;
	
	// Constructor por defecto
	Jugador() : nombre(""), contraseña(""), puntos(1000), ip(sf::IpAddress::Any), port(0) {}

	// Constructor con parámetros
	Jugador(const std::string& nombre, const std::string& contraseña, int puntos, sf::IpAddress ip, unsigned short port)
		: nombre(nombre), contraseña(contraseña), puntos(puntos), ip(ip), port(port) {}
};


class Servidor_Servicios {


	public:

		void iniciarServidor();// Inicia el servidor y maneja conexiones
		void procesarMensajes();// Procesa los mensajes recibidos de los clientes
		
		
		

		void manejarLogin(const std::string& receivedData, sf::IpAddress ip, unsigned short port);// Maneja solicitudes de login
		void manejarRegister(const std::string& receivedData, sf::IpAddress ip, unsigned short port);// Maneja registros de nuevos usuarios
		void manejarMatchmaking(const std::string& data, sf::IpAddress ip, unsigned short port);// Maneja la búsqueda de partidas
		void manejarVictoria(const std::string& data, sf::IpAddress ip, unsigned short port);// Registra la victoria de un jugador y actualiza el ranking
		void emparejarJugadores(); // Empareja jugadores y asigna servidores de juego

		
		void notificarPartida(Jugador& jugador1, Jugador& jugador2);//Notifica a los clientes que se ha encontrado partida
		
		sf::UdpSocket UdpSocket;
		std::queue<Jugador> colaAmistosa;
		std::vector<Jugador>colaCompetitiva;
	
		std::map<std::string, Jugador> usuarios; // Base de datos Jugadores en memoria

		//Ip adress Gameplay
		sf::IpAddress serverAddressGameplay = sf::IpAddress::LocalHost;
		//std::optional<sf::IpAddress> serverAddressGameplay = sf::IpAddress::resolve("192.168.1.100");
		unsigned short serverPortGameplay = 55000;

		

};
