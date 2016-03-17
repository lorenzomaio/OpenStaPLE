#!/usr/bin/python

#modify this to change compiler, linker etc options.
PGIcls = 'COMPILER=pgcc\n\
COMPILER_FLAGS=-O3 -acc -Minfo=accel -v -ta=tesla:cc35,cuda7.0 \n\
LINKER_FLAGS=-acc  -Minfo=accel -O3 -v -ta=tesla:cc35,cuda7.0 \n' 

PGIclsSLOW ='COMPILER=pgcc\n\
COMPILER_FLAGS=-O0 \n\
LINKER_FLAGS=-O0 \n' 

GNUcls = 'COMPILER=gcc\n\
COMPILER_FLAGS=-O3 -std=gnu99\n\
LINKER_FLAGS=-lm\n' 


MPIcls = 'COMPILER=mpicc\n\
COMPILER_FLAGS=-O3 -std=gnu99\n\
LINKER_FLAGS=-lm\n' 


compiler_linker_settings = ''

make_rgen_string ='\n\
\
rgen: \n\
\tcd ../tools/ && $(MAKE) rgen\n\
\tcp ../tools/rgen ./run/\n'






import os.path as path
from sys import exit,argv,stderr,stdout


# General philosophy: build a graph of files, with directed links, 
# which depics the dependences in the code as described with #includes.
# Loops are not allowed.
# there are two types of links:
#
# - ".h" dependences: read by searching for '#include' directives
#         |  a modification of this kind of dependence should trigger 
#         |  recompilation of the file associated to the depending 
#         |  node. Only direc 1-link dependences are used, make will
#         |  automagically take care of the indirect >1-link dependences, 
#         |  up to the leaves of the tree ('all dependences')
#
# - ".c" dependences: created from .h dependences. Only the set of 
#         | 'leaves' (all the indirect dependences) are used here 
#         | and they play a role during the linking phase.
#                     


# all nodes in the graph (all files)
allnodesdict = dict() # abspath filename -> file_node
main_files = [] # file containing main()

# The following function scans the file to find .h files that are 
# included in the project. 
# NOTICE: 
# at present, all header inclusion statements must be written
# in a particular way, for example
#  
#  #include "../this/is/an/example.h"
#  #include "./notice/the/filename/beginning/with_a_dot.h"
#  #include "this_file_wont_be_considered_as_part_of_the_project.h"
#
# that is, file included using a filename non starting with a dot
# will be considered as part of some "standard library" and not checked.


def header_scanner(filename): # only headers!
#   stderr.write("Opening file " + filename + "\n" )
    f = open(filename)
    filesfound = []
    standards = [] # standard libraries that are not in the repo
                   # and should not be cheked by make

    for line in f.readlines():
        if 'main(' in line and ')' in line and filename not in main_files:
            main_files.append(filename)
        if '#include' in line :
            init_filename_position = line.find('<')
            end_filename_position = line.rfind('>')
            if init_filename_position == -1:
                init_filename_position = line.find('"')
                end_filename_position = line.rfind('"')
            if init_filename_position != -1 and end_filename_position != -1:
                filefound = line[init_filename_position+1:end_filename_position]
#               stderr.write("  File found : " + line + ' \n')
#               stderr.write("  File found : -" + filefound + '- \n')
                if (filefound[0] is '.')  and ('.h' in filefound):
                    filesfound.append(filefound)
                else:
#                   stderr.write("  " + filefound +' is not in this package, not adding it.\n')
                    standards.append(filefound) # libraries like stdio.h
            else:
                stderr.write( "  #include line incorrect:" + filename + ' ' + line + '\n')

    return filesfound,standards


def fill_allnodesdict():
    complete = False
    while not complete:
        complete = True
        names_to_add = []
        for filename in allnodesdict:
            for dependence in allnodesdict[filename].direct_dependences:
                if dependence not in allnodesdict:
                    names_to_add.append(dependence)
            for dependence in allnodesdict[filename].direct_dependences_c:
                if dependence not in allnodesdict:
                    names_to_add.append(dependence)
        for name in names_to_add:
            complete = False
            allnodesdict[name] = file_node(name)

def set_all_dependences():
    for filename in allnodesdict:
        allnodesdict[filename].get_all_dependences()

class file_node:
    # to use after fill_allnodesdict()
    def get_all_dependences_raw(self,ancestors): # raw means with repetitions
                                       # only .h files
        ancestors += [ self.name ]
        res = list(self.direct_dependences)
        for dep in self.direct_dependences:
            if dep not in ancestors:
                son = allnodesdict[dep]
                son_dependences = son.get_all_dependences_raw(ancestors)
                if son_dependences is not None:
                    res = res + son_dependences
        return res
    # to use after fill_allnodesdict()
    def get_all_dependences_c_raw(self, ancestors): # with repetitions
                                                    # all c files.
        res = list(self.direct_dependences_c)
        ancestors += [ self.name ]
        for dependence in self.direct_dependences_c:
            if dependence not in ancestors:
                son = allnodesdict[dependence]
                son_dependences_c = son.get_all_dependences_c_raw(ancestors)
                if son_dependences_c is not None:
                    res = res + son_dependences_c
        return res
    # to use after fill_allnodesdict()
    def get_all_standard_dependences_raw(self,ancestors):  # with repetitions
                                          # all 'standard library'
                                          # not included in the project
        res = list(self.direct_standard_dependences)
        ancestors += [ self.name ]
        for dep in self.direct_dependences:
            son_dependences = allnodesdict[dep].get_all_standard_dependences_raw(ancestors)
            if son_dependences is not None:
                res = res + son_dependences
        return res

    def __init__(self,filename):
        self.direct_standard_dependences = [] #libraries like stdio.h
        self.direct_dependences_relative = [] # relative path
        self.direct_dependences = []
        self.direct_dependences_c = []
        self.csource_dependences_relative = []
        self.all_dependences = []
        self.all_dependences_c = []
        self.all_standard_dependences = []
        self.name = path.abspath(filename)
        #stderr.write("Scanning " + filename + "...\n" )
        self.direct_dependences_relative, self.direct_standard_dependences\
              = header_scanner(filename)
        for dependence in self.direct_dependences_relative:
            # gets the absolute path for robustness
            dependencem = path.abspath(path.dirname(self.name)+ '/'+dependence)
            if dependencem == self.name :
                stderr.write("%s has itself as a dependence, fix this!\n" % self.name)
                exit(1)
            self.direct_dependences.append(dependencem)
            dependencemc = dependencem[:-2] + '.c' 
            if path.exists(dependencemc):
                self.direct_dependences_c.append(dependencemc)

    def get_all_dependences(self):
        self.all_dependences_raw = self.get_all_dependences_raw([]);
        self.all_standard_dependences_raw = self.get_all_standard_dependences_raw([]);
        self.all_dependences_c_raw = self.get_all_dependences_c_raw([])
        if self.all_standard_dependences_raw is not None:
            for dependence in self.all_standard_dependences_raw:
                if dependence not in self.all_standard_dependences:
                    self.all_standard_dependences.append(dependence)
        if self.all_dependences_raw is not None:
            for dependence in self.all_dependences_raw:
                if dependence not in self.all_dependences:
                    self.all_dependences.append(dependence)
        if self.all_dependences_c_raw is not None:
            for dependence in self.all_dependences_c_raw:
                if dependence not in self.all_dependences_c:
                    self.all_dependences_c.append(dependence)


    def showtree(self,n): # debugging/visualizing
        prestring = ' ' * n
        for dependence in self.direct_dependences:
            son = allnodesdict[dependence]
            print(prestring+dependence)
            son.showtree(n+1)
    def showtree_c(self,n,ancestors): # debugging/visualizing
        prestring = ' ' * n
        for dependence in self.direct_dependences_c:
            if dependence not in ancestors:
                son = allnodesdict[dependence]
                print(prestring+dependence)
                son.showtree_c(n+1,ancestors + [ dependence ])


    def generate_make_string(self):
        makestring = ''
        if '.h' in self.name:
            makestring += self.name + " :"
        elif '.c' in self.name:
            makestring += path.basename(self.name)[:-2] + '.o : ' + self.name
        else:
            stderr.write("Filename " + self.name + " not valid.\n")
            return ''

        for dependence in self.direct_dependences:
            makestring += ' ' + dependence
        makestring += '\n\t'
        if '.h' in self.name:
            makestring += 'touch ' + self.name + '\n'
        elif '.c' in self.name:
            makestring += '$(COMPILER) -c $(COMPILER_FLAGS) ' +\
                    self.name + '\n\n'
        else:
            stderr.write("Filename " + self.name + " not valid.\n")
            return ''
        return makestring

    def generate_linker_string(self):
        linkstring = ''
        allobjects = ''
        exename = path.basename(self.name)[:-2]
        linkstring = exename + " : "
        allobjects += exename + '.o'  
        for filename in self.all_dependences_c:
            allobjects += ' ' + path.basename(filename)[:-2] + '.o'
        linkstring += allobjects + '\n'
        linkstring +='\t$(COMPILER) -o '+ exename + ' ' + allobjects +' $(LINKER_FLAGS)\n'
        linkstring +='\tif ! [ -d run ] ; then mkdir run; fi ; cp ' 
        linkstring += exename + ' run/\n\n' 
        return linkstring


def generate_makefile_from_main(inputfiles):

    res = ''
    res += compiler_linker_settings + '\n'

    common_linking_string = ''
    
    for filename in inputfiles: 
        filenamem = path.abspath(filename)
        allnodesdict[filenamem] = file_node(filenamem,)

    fill_allnodesdict()
    set_all_dependences()


    for filename in allnodesdict :
        if '.c' in filename:
            common_linking_string += ' ' + path.basename(filename)[:-2] + '.o'
        node = allnodesdict[filename]
        makestring = node.generate_make_string()
        res += makestring
        
    for filename in main_files:
        res += allnodesdict[filename].generate_linker_string()


    makeclean_string='clean:\n\trm -f *.o'
    for filename in main_files:
        exename = path.basename(filename)[:-2]
        makeclean_string += ' ' + exename

    
    makeall_string='all: '
    for filename in main_files:
        exename = path.basename(filename)[:-2]
        makeall_string += ' ' + exename

 

    return res, makeclean_string, makeall_string


if __name__ == '__main__':

    if 'PGISLOW' not in argv and 'GNU' not in argv and 'PGI' not in argv and 'MPI' not in argv:
        stderr.write("Please specify one compiler: either PGISLOW, GNU, PGI or MPI\n")
        exit(1)

    clsset = False
    if 'PGISLOW' in argv : 
        compiler_linker_settings = PGIclsSLOW;
        argv.remove('PGISLOW')
        clsset = True
    if 'GNU' in argv :
        if clsset :
            stderr.write("Please specify one compiler: either PGISLOW, GNU or PGI\n")
            argv.remove('GNU')
            exit(1)
        else:
            compiler_linker_settings = GNUcls;
            argv.remove('GNU')
            clsset = True
    if 'PGI' in argv :
        if clsset :
            stderr.write("Please specify one compiler: either PGISLOW, GNU or PGI\n")
            argv.remove('PGI')
            exit(1)
        else:
            compiler_linker_settings = PGIcls;
            argv.remove('PGI')
            clsset = True
    if 'MPI' in argv :
        if clsset :
            stderr.write("Please specify one compiler: either PGISLOW, GNU or PGI\n")
            argv.remove('MPI')
            exit(1)
        else:
            compiler_linker_settings = MPIcls;
            argv.remove('MPI')
            clsset = True




    if not path.exists("lattice_dimensions.h"):
        stderr.write("generating lattice_dimensions.h\n")
        f = open("lattice_dimensions.h", 'w')
        ldim_string = '''
// lattice dimensions
#define LOC_N0  8
#define LOC_N1  8
#define LOC_N2  8
#define LOC_N3  8

// MULTIDEVICE
#define NRANKS_D0 1
#define NRANKS_D1 1
#define NRANKS_D2 1
#define NRANKS_D3 1
'''
        f.write(ldim_string)
        f.close()





    makemains,makeclean,makeall = generate_makefile_from_main(argv[1:])

    makeclean += ' rgen\n'
    makeall += ' rgen\n'


    stdout.write(makemains)
    stdout.write(make_rgen_string)
    stdout.write(makeall)
    stdout.write(makeclean)

    




