project('tutorial', 'c')


cc = meson.get_compiler('c')

dependencies = [
dependency('gtk+-3.0'),
dependency('cairo', required : true),
dependency('openmp'), 
cc.find_library('m', required : true)
]


extra_args = [] #'-D DEBUG']

src = ['utils.c', 'main.c']
executable('demo', src, dependencies : dependencies, c_args : extra_args)