# Commande de délétion
RM=rm
# Flags de délétion
RMFLAGS=-rf

# Bibliothèques supplémentaires
LIB=bluetooth pthread rt readline ncurses menu sphero

# Répertoires de bibliothèques
LIBDIR?=..

# Commande écho
ECHO=@echo

# Nom de la bibliothèque
EXECNAME=sphero-cli

# Dossiers d'include perso
INCDIR=src

# Dossier d'include externes
EXTINCDIR?=

# Dossier sources
SRCDIR=src

# Dossier objets
OBJDIR=obj

# Dossier où sont mises les dépendances
DEPDIR=dep
df=$(DEPDIR)/$(*F)

SRC=$(shell find $(SRCDIR) -type f -name *.cpp | sed -e "s/$(SRCDIR)\///")
OBJ=$(SRC:.cpp=.o)

CLEAR=clean

MAKEDEPEND = g++ $(addprefix -I, $(EXTINCDIR)) -I$(INCDIR) -o $(df).d -std=c++11 -MM $< #Pour calculer les dépendances

#Compilateur
CC=g++
#Options du compilateur
CCFLAGS+=-Wall -fPIC -fpermissive -Wextra -Woverloaded-virtual -std=c++11 -I$(INCDIR) $(addprefix -I, $(EXTINCDIR)) $(addprefix -l, $(LIB)) -c -pthread -g

EL=g++ #Éditeur de liens
ELFLAGS= -Wl,-rpath=.. -g


DSHARP?=FALSE

MAP?=FALSE

PROF?=FALSE

ifneq ($(DSHARP),FALSE)
    CCFLAGS+= -DSHARP 
endif

ifneq ($(MAP),FALSE)
    CCFLAGS+= -DMAP 
endif

ifneq ($(PROF),FALSE)
    CCFLAGS+= -pg
    ELFLAGS+= -pg
endif


.PHONY: $(CLEAR)
.PHONY: ALL

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp 
	@mkdir -p $(DEPDIR);
	@mkdir -p $(OBJDIR);
	@$(MAKEDEPEND); \
		cp $(df).d $(df).P;\
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	            -e '/^$$/ d' -e 's/$$/ :/' < $(df).d >> $(df).P;\
        	sed -i '1s/^/$(OBJDIR)\//' $(df).P;
		@rm -f $(df).d
	@$(ECHO) "Compilation de $<"
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ $<


ALL: $(EXECNAME)
	
$(EXECNAME): $(addprefix $(OBJDIR)/, $(OBJ)) 
	$(ECHO) "Fabrication de l'application"
	$(EL) -o $(EXECNAME) $(addprefix $(OBJDIR)/, $(OBJ)) $(ELFLAGS) $(addprefix -L, $(LIBDIR)) $(addprefix -l, $(LIB)) 

#Fichiers de dépendance
-include $(SRC:%.cpp=$(DEPDIR)/%.P)

$(CLEAR):
	$(RM) $(RMFLAGS) $(OBJDIR)/* $(DEPDIR)/*.P $(EXECNAME) 
