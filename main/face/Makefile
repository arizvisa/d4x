all:lod.o list.o colors.o addd.o graph.o edit.o log.o prefs.o lmenu.o \
	buttons.o about.o saveload.o misc.o columns.o dndtrash.o \
	passface.o mywidget.o myclist.o fsface.o fsched.o filtrgui.o qtree.o

clean:
	-rm -f *.o *~

Makefile.dep:
	$(CXX) $(CXXFLAGS) -MM *.cc > Makefile.dep

-include Makefile.dep
