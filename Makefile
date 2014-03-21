NAME 		= bin/mind

SRCDIR		= src/

COMMON		= $(SRCDIR)irc.cpp	\
		$(SRCDIR)socket.cpp	\
		$(SRCDIR)user.cpp	\
		$(SRCDIR)conf.cpp	\
		$(SRCDIR)utils.cpp	\
		$(SRCDIR)logger.cpp


SRC 		= $(SRCDIR)main.cpp	\
		$(SRCDIR)activity.cpp	\
		$(COMMON)


MODULEDIR	= modules/

COMMON_OBJ	= $(COMMON:.cpp=.o)

OBJ 		= $(SRC:.cpp=.o)

CXXFLAGS 	+= -W -Wall $(XMLCFLAGS) -I. -I.. -I$(SRCDIR) -I$(MODULEDIR) -O3 $(shell xml2-config --cflags) -fPIC -ggdb -rdynamic
LDFLAGS		= -lpthread -lcurl -ldl $(shell xml2-config --libs) -ldb_cxx -lcrypto
LIB_FLAGS	= -fPIC

CXX 		= g++

MODULE_SRC =	$(wildcard $(MODULEDIR)*.cpp)

MODULE_OBJ =	$(MODULE_SRC:.cpp=.so)

all: 		$(NAME) modules

$(NAME): 	$(OBJ)
		@test -d bin || mkdir -p bin
		$(CXX) -o $(NAME) $(CXXFLAGS) $(OBJ) $(LDFLAGS)

.cpp.o:
		$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
		@rm -f $(OBJ) $(MODULEDIR)*.o
		@rm -f *~
		@rm -f \#*#

distclean:	clean
		@rm -f $(NAME)
		@rm -f $(MODULEDIR)*.so

modules:	$(NAME) $(MODULE_OBJ)
		@test -d db || mkdir db

%.so: %.cpp
		$(CXX) -c $< -o $*.o $(LIB_FLAGS) $(CXXFLAGS)
		$(CXX) -shared -o $@ $*.o $(COMMON_OBJ) $(CXXFLAGS) $(LDFLAGS) $(LIB_FLAGS)

