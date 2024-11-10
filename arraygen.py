import re

ws = re.compile(r'\s+')
lf = re.compile(r'\n+')

ARR_TYPE = 'vec'
header = 'template<typename T, int N> constexpr $ARR<$OUT, N> '
bin_templ = header+'''
operator$OP($ARR<T,N> a, $ARR<T,N> b){
  $ARR<$OUT, N> r{};
  for(int i=0;i<N;i++) r[i] = a[i] $OP b[i];
  return r;
}
'''
bin_scalar_templ_a = header+'''
operator$OP($ARR<T,N> a, T s){
  $ARR<$OUT, N> r{};
  for(int i=0;i<N;i++) r[i] = a[i] $OP s;
  return r;
}
'''
bin_scalar_templ_b = header+'''
operator$OP(T s, $ARR<T,N> a){
  $ARR<$OUT, N> r{};
  for(int i=0;i<N;i++) r[i] = s $OP a[i];
  return r;
}
'''
unary_templ = header+'''
operator$OP($ARR<T,N> a){
  $ARR<$OUT, N> r{};
  for(int i=0;i<N;i++) r[i] = $OP a[i];
  return r;
}
'''

def arith_bin(op):
    return '\n'.join([bin_templ, bin_scalar_templ_a, bin_scalar_templ_b]).replace('$OP', op).replace('$OUT', 'T')

def arith_unary(op):
    return unary_templ.replace('$OP', op).replace('$OUT', 'T')

def logic_bin(op):
    return bin_templ.replace('$OP', op).replace('$OUT', 'bool')

def logic_unary(op):
    return unary_templ.replace('$OP', op).replace('$OUT', 'bool')

operators = {
    '+': [arith_bin, arith_unary],
    '-': [arith_bin, arith_unary],
    '*': [arith_bin],
    '/': [arith_bin],
    '%': [arith_bin],
    '&': [arith_bin],
    '|': [arith_bin],
    '^': [arith_bin],
    '~': [arith_unary],
    '&&': [logic_bin],
    '||': [logic_bin],
    '==': [logic_bin],
    '!=': [logic_bin],
    '>=': [logic_bin],
    '<=': [logic_bin],
    '>': [logic_bin],
    '<': [logic_bin],
    '!': [logic_unary],
}

decls = []
for op, funcs in operators.items():
    decls += [ws.sub(' ', f(op)).strip() for f in funcs]

decls.insert(0, '''template<typename T, int N>
struct $ARR {
  T data[N];
  constexpr T& operator[](int i){ return data[i]; }
  constexpr T const& operator[](int i) const { return data[i]; }
};\n
''')

decls = lf.sub('\n', '\n'.join(decls).replace('$ARR', ARR_TYPE))
decls = '\n'.join(filter(lambda x: len(x.strip()) > 0, decls.replace('template', '\ntemplate').split('\n')))
print(decls)
