

(module
  (type $ret-i32 (func (param i32) (result i32)))
  (func $host.test (import "host" "test") (param i32) (result i32))
  (func $host.test2 (import "host" "test2") (param i32) (result i32))
  (memory 1)
  (table 3 3 anyfunc)
  (func $local.test (export "test") (param i32) (result i32)
    (get_local 0)
    (call $host.test)
    (i32.const 2)
    (i32.add)
  )
  (func (export "test.indirect") (param i32 i32) (result i32)
    (get_local 0)
    (get_local 1)
    (call_indirect (type $ret-i32))
  )
  (func $dummy (param i32) (result i32) (i32.const -67))
  (func (export "test.local-call") (param i32) (result i32)
    (get_local 0)
    (call $local.test)
    (i32.const 3)
    (i32.mul)
  )
  (elem (i32.const 0) 0 1 2)
)