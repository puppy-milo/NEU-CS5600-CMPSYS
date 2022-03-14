# Fail cases
empty_enqueue_test_vector = [
    {
        'in': 'enqueue\n',
        'out': 'ERROR'
    },
    {
        'in' : 'enqueue 5\ndequeue\nenqueue\n',
        'out': '5ERROR'
    },
    {
        'in' : 'dequeue\nenqueue\n',
        'out': 'NULLERROR'}
]

# Cause a huge linked list to see if the memory handling is correct
large_enqueue_op = [
    {
        'in': 'enqueue 5\n'*1024 + 'dequeue\n'*1024 + 'dequeue\n',
        'out': '5' * 1024 + 'NULLfinally: '
    }
]

# Regular test case.
regular_pass_test_vector = [
    {
        'in': 'enqueue ABC\ndequeue\nenqueue HIJ\nenqueue DEF\ndequeue\ndequeue\ndequeue\n',
        'out': 'ABCHIJDEFNULLfinally: '
    }
]

# Regular case with print linked list after
print_list_pass_test_vector = [
    {
        'in': "enqueue ABC\ndequeue\nenqueue BDE\ndequeue\ndequeue\nenqueue CS\nenqueue 5600\n",
        'out': 'ABCBDENULLfinally: [CS]->[5600]'
    }
]
