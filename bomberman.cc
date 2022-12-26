#include "bomberman.hh"
#include <stdexcept>
#include <algorithm>

std::ostream & operator<<(std::ostream & os, position const & p) {
	os << "(" << p.x() << "," << p.y() << ")";
	return os;
}

entite::identifiant entite::_cpt(0);
unsigned int bombe::_duree=10;

std::ostream & operator<<(std::ostream & os, entite const & e) {
	e.sortieflux(os);
	return os;
}

bool bombe::decompte() {
	if (_decompte == 0)
		if (_etat == etat::explosion)
			return true;
		else {
			_etat = etat::explosion;
			_decompte = _duree;
		}
	else
		--_decompte;
	return false;
}

plateau::plateau(position const & t)
	:_contenu(), _taille(t) {
	auto nb(static_cast<std::size_t>(t.x())*static_cast<std::size_t>(t.y()));
	for (std::size_t i=0; i<nb; ++i)
		_contenu.push_back(std::unique_ptr<entite>());
}

void plateau::ajouter(std::unique_ptr<entite> e, position const & p) {
	auto & rech(recherche(p));
	rech = std::move(e);
}

entite const & plateau::acces(entite::identifiant id) const {
	auto f(std::find_if(_contenu.begin(), _contenu.end(), [id](auto const & i){ return i && (i->id() == id); }));
	if (f != _contenu.end())
		return (**f);
	else
		throw std::domain_error("Identifiant inconnu");
}

entite::identifiant plateau::idmax() const {
	entite::identifiant resultat(0);
	for (auto const & i : _contenu) {
		if ((resultat == 0) || (i && i->id() > resultat))
			resultat = i->id();
	}
	return resultat;
}

bool plateau::deplacerjoueur(position const & src, position const & dst) {
	try {
		auto & srcp(recherche(src));
		if (!srcp)
			return false;
		if (!dynamic_cast<joueur const *>(srcp.get()))
			return false;
		auto & dstp(recherche(dst));
		if (dstp && !dynamic_cast<bombe const *>(dstp.get()))
			return false;
		dstp = std::move(srcp);
		return true;
	}
	catch (std::invalid_argument const &) {
		return false;
	}
}

void plateau::toursuivant() {
	std::list<position> explosions;
	for (coord y=0; y<_taille.y(); ++y) {
		for (coord x=0; x<_taille.x(); ++x) {
			auto & i(recherche(position(x,y)));
			if (i) {
				auto bp(dynamic_cast<bombe *>(i.get()));
				if (bp) {
					if (bp->decompte())
						i = nullptr;
					else
						if (bp->accesetat() == bombe::etat::explosion)
							explosion(position(x,y), bp->largeur(), explosions);
				}
			}
		}
	}
	std::cout << "Explosions : ";
	for (auto const & i : explosions)
		std::cout << i << " ";
	std::cout << "\n";
	std::cout << "Joueurs tués : ";
	for (auto const & i : explosions) {
		auto & ent(recherche(i));
		if (ent) {
			auto jr(dynamic_cast<joueur *>(ent.get()));
			if (jr) {
				std::cout << jr->nom() << " ";
				ent = nullptr;
			}
		}
	}
	std::cout << "\n";
}

bool plateau::valide(position const & p) const {
	return (p.x() >= 0) && (p.y() >= 0) && (p.x() < _taille.x()) && (p.y() < _taille.y());
}

std::unique_ptr<entite> const & plateau::recherche(position const & p) const {
	if (!valide(p))
		throw std::invalid_argument("Position invalide");
	return _contenu[p.y()*_taille.x()+p.x()];
}

std::unique_ptr<entite> & plateau::recherche(position const & p) {
	if (!valide(p))
		throw std::invalid_argument("Position invalide");
	return _contenu[p.y()*_taille.x()+p.x()];
}

void plateau::explosion(position const & p, coord largeur, std::list<position> & resultat) const {
	for (auto dx : {-1, 0, 1}) // Solution pas forcément géniale, mais courte à écrire
		for (auto dy : {-1, 0, 1})
			if (((dx == 0) && (dy != 0)) || ((dx != 0) && (dy ==0)))
				for (coord l(0); l<largeur; ++l) {
					position p2(p.x()+dx*l, p.y()+dy*l);
					try {
						auto const & ent(recherche(p2));
						if (!ent || !dynamic_cast<obstacle const *>(ent.get())) {
							if (std::find(resultat.begin(), resultat.end(), p2) == resultat.end())
								resultat.push_back(p2);
						}
						else
							break;
					}
					catch (std::invalid_argument const &) {
						break;
					}
				}
}

void plateau::sortieflux(std::ostream & os) const {
	for (coord y=0; y<_taille.y(); ++y) {
		for (coord x=0; x<_taille.x(); ++x) {
			auto const & i(recherche(position(x,y)));
			if (i)
				os << i->symbole();
			else
				os << " ";
		}
		os << "\n";
	}
}

std::ostream & operator<<(std::ostream & os, plateau const & p) {
	p.sortieflux(os);
	return os;
}
std::size_t nbjoueurs(plateau const &p) {
	std::size_t nb(0);
	for (entite::identifiant i=0; i<p.idmax(); ++i) {
		try {
			dynamic_cast<joueur const &>(p[i]);
			++nb;
		}
		catch (...) {
		}
	}
	return nb;
}

int main() {
	const int L(30), H(20);
	plateau p(position(L, H));
	for (coord x=0; x<L; ++x) {
		p.ajouter(std::make_unique<obstacle>(), position(x,0));
		p.ajouter(std::make_unique<obstacle>(), position(x,H-1));
	}
	for (coord y=0; y<H; ++y) {
		p.ajouter(std::make_unique<obstacle>(), position(0,y));
		p.ajouter(std::make_unique<obstacle>(), position(L-1,y));
	}
	p.ajouter(std::make_unique<joueur>("1-Player"), position (5,5));
	p.ajouter(std::make_unique<joueur>("2-Player"), position (6,6));
	p.ajouter(std::make_unique<obstacle>(), position (3,8));
	p.ajouter(std::make_unique<joueur>("3-Player"), position (3,9));
	p.ajouter(std::make_unique<bombe>(4), position (3,6));
	// Test d'ajout à une position invalide
	try {
		p.ajouter(std::make_unique<obstacle>(), position (L,H-1));
		std::cout << "!! Pas normal : pas d'exception";
	}
	catch (std::invalid_argument const &) {
		std::cout << "Normal : Exception\n";
	}
	// Écrasement d'une entité par une autre (bombe en bas à droite)
	p.ajouter(std::make_unique<bombe>(1), position (L-1,H-1));
	p.ajouter(std::make_unique<joueur>("4-Player"), position (3,10));
	p.ajouter(std::make_unique<bombe>(1), position (3,11));
	std::cout << "Deplacement en 3,11 : OK " << p.deplacerjoueur(position(3,10), position(3,11)) << "\n";
	std::cout << "Deplacement en 0,0 : NOK " << p.deplacerjoueur(position(3,11), position(0,0)) << "\n";

	std::cout << "Nombre joueurs " << nbjoueurs(p)<< "\n";
	for (unsigned int i=0; i<30; ++i) {
		std::cout << p;
		p.toursuivant();
	}
	return 0;
}
