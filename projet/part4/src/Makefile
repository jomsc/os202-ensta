# Inclure les configurations spécifiques à la plateforme
include Make_linux.inc
#include Make_msys2.inc
#include Make_osx.inc

# Compilateur MPI
CXX = mpic++

# Drapeaux de compilation
CXXFLAGS = -std=c++17 -Wall
ifdef DEBUG
CXXFLAGS += -g -O0 -fbounds-check -pedantic -D_GLIBCXX_DEBUG -fsanitize=address
else
CXXFLAGS += -O3 -march=native
endif

# Bibliothèques à lier
LIB = -lSDL2

# Nom de l'exécutable
ALL = simulation

# Règle par défaut
default: help

# Règle pour créer l'exécutable
all: $(ALL)

# Règle pour nettoyer les fichiers générés
clean:
	@rm -fr *.o $(ALL) *~ *.d

# Règle pour compiler les fichiers sources en fichiers objets
.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# Règle pour lier les fichiers objets et créer l'exécutable
simulation: display.o model.o simulation.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB)

# Règle pour afficher l'aide
help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

# Règle pour générer des fichiers HTML à partir de Markdown
%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css --metadata pagetitle="OS202 - TD1" -o $@

# Inclure les dépendances générées automatiquement
-include *.d
