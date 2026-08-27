foreach name : ['compression', 'simulated', 'solution_ctor']
    exe = executable('test_' + name, 'test_' + name + '.cc',
        dependencies: djinni_dep,
    )
    test(name, exe, args: [dumas_dataset], timeout: 60)
endforeach
