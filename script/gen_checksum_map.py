res = ''
for i in range(0, 1000):
    res += '    case %i: return "%s";\n' % (i, str(i).zfill(3))
res += '    default: assert(false);'

result = '''
#pragma once

namespace atrfix {

inline const char* checksum(int i) {
  switch(i) {
%s
  }
}
   
}
''' % res

print(result)
