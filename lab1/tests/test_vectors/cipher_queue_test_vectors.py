# Fail cases
empty_enqueue_test_vector = [
    {
        'in': 'encode_enqueue\n',
        'out': 'ERROR'
    },
    {
        'in' : 'encode_enqueue 1 ABC\ndequeue\nenqueue\n',
        'out': 'BCDERROR'
    },
    {
        'in' : 'dequeue\nenqueue\n',
        'out': 'NULLERROR'}
]

# Cause a huge linked list to see if the memory handling is correct
large_enqueue_op = [
    {
        'in': 'encode_enqueue 1 ABC\n'*1024 + 'dequeue\n'*1024 + 'dequeue\n',
        'out': 'BCD' * 1024 + 'NULLfinally: '
    }
]

# Regular test case.
regular_pass_test_vector = [
    {
        'in': 'encode_enqueue 1 ABC\ndequeue\nencode_enqueue 1 ABC\ndequeue_decode 1\nencode_enqueue 1 A+C\ndequeue_decode 1\nenqueue CS\nenqueue 5600\nencode_enqueue 1 CS\nencode_enqueue 1 5600\n',
        'out': 'BCDABCILLCHARfinally: [CS]->[5600]->[DT]->[6711]'
    }
]

