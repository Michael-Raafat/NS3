## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    obj = bld.create_ns3_program('system', ['netanim','core', 'point-to-point', 'internet', 'applications'])
    obj.source = 'system.cc'

