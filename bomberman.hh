#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <list>

using coord=signed short;
class position {
	public:
		position(coord x, coord y):_x(x), _y(y) {}
		coord x() const { return _x; }
		coord y() const { return _y; }
		bool operator==(position const & p) const { return (_x==p._x) && (_y==p._y); }
	private:
		coord _x;
		coord _y;
};

std::ostream & operator<<(std::ostream & os, position const & p);

class entite {
	public:
		using identifiant = unsigned int;
	public:
		entite(): _id(++_cpt) {}
		virtual ~entite() =default;
		virtual char symbole() const =0;
		identifiant id() const { return _id; }
		virtual void sortieflux(std::ostream & os) const { os << symbole(); }
	private:
		identifiant _id;
		static identifiant _cpt;
};

std::ostream & operator<<(std::ostream & os, entite const & e);

class joueur: public entite {
	public:
		joueur(std::string const & n): entite(), _nom(n) {}
		char symbole() const { return _nom[0]; }
		void sortieflux(std::ostream & os) const override { entite::sortieflux(os); os << " " << _nom; }
		std::string const & nom() const { return _nom; }
	private:
		std::string _nom;
};

class bombe: public entite {
	public:
		enum class etat {
			decompte,
			explosion,
		};
		bombe(coord largeur): entite(), _decompte(_duree), _largeur(largeur), _etat(etat::decompte) {}
		char symbole() const { if (_etat == etat::decompte) return 'B'; else return 'E'; }
		void sortieflux(std::ostream & os) const override { entite::sortieflux(os); os << " " << _decompte; }
		etat accesetat() const { return _etat; }
		coord largeur() const { return _largeur; }
		bool decompte();
		static void modifierduree(unsigned int nd) { _duree = nd; }
	private:
		static unsigned int _duree;
		unsigned int _decompte;
		coord _largeur;
		etat _etat;
};

class obstacle: public entite {
	public:
		obstacle(): entite() {}
		char symbole() const { return '#'; }
};

class plateau {
	public:
		plateau(position const & t);
		plateau(plateau const & p) =delete;
		plateau & operator=(plateau const & p) =delete;
		void ajouter(std::unique_ptr<entite> e, position const & p);
		entite const & acces(entite::identifiant id) const;
		entite const & operator[](entite::identifiant id) const { return acces(id); }
		entite::identifiant idmax() const;
		void sortieflux(std::ostream & os) const;
		bool deplacerjoueur(position const & src, position const & dst);
		void toursuivant();
	private:
		bool valide(position const & p) const;
		std::unique_ptr<entite> const & recherche(position const & p) const;
		std::unique_ptr<entite> & recherche(position const & p);
		void explosion(position const & p, coord largeur, std::list<position> & resultat) const;
	private:
		std::vector<std::unique_ptr<entite>> _contenu;
		position _taille;
};

std::ostream & operator<<(std::ostream & os, plateau const & p);
std::size_t nbjoueurs(plateau const &p);
