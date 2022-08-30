project('tutorial', 'c')

dependencies = [
dependency('gtk+-3.0'),
dependency('cairo', required : true),
dependency('openmp')
]

src = ['main.c']
executable('demo', src, dependencies : dependencies)