project('tracker-sparqld', 'c',
  version : '0.0.1',)

sources = [
  'src/conf.c',
  'src/log.c',
  'src/request.c',
  'src/query.c',
  'src/sparqld.c'
  ]

config_dep = dependency('libconfig')
event_dep = dependency('libevent')
tracker_sparql_dep = dependency('tracker-sparql-2.0')

sparqld = executable('tracker-sparqld',
  sources,
  dependencies : [config_dep, event_dep, tracker_sparql_dep],
  install : true
)