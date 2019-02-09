- change definition of ENTRY in lilyDefaultEnvironment.hpp to assume
  symbols that don't need quoting (faster startup?)
- use _lilyFold and avoid wrapping for nary ops with determined result
  type with a machine representation
