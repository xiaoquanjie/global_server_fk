#! /usr/bin/env python
# coding=utf-8

import os
import platform
import xml.dom.minidom
from xml.etree import ElementTree as ET
from xml.etree.ElementTree import ElementTree

def generator_build_file():
    file_str = "cc_library(\n"
    file_str += "\tname = 'proto',\n"
    file_str += "\tsrcs = [\n"

    files = os.listdir('./src/')
    for f in files:
        if not f.endswith('.cc'):
            continue
        file_str += "\t\t'" + "src/" + f + "',\n"

    file_str += "\t],\n"
    file_str += "\tdeps = [\n"
    file_str += "\t\t'//google:protobuf',\n"
    file_str += "\t],\n"
    file_str += ")"

    f = open('BUILD', mode='w')
    f.write(file_str)

def append_include_node(dom_tree, item_group, f):
    child = dom_tree.createElement('ClInclude')
    child.setAttribute("Include", f)
    item_group.appendChild(child)

def append_compile_node(dom_tree, item_group, f):
    child = dom_tree.createElement('ClCompile')
    child.setAttribute("Include", f)
    item_group.appendChild(child)

def append_include_filter_node(dom_tree, item_group, f):
    child = dom_tree.createElement('ClInclude')
    child.setAttribute("Include", f)
    node_child = dom_tree.createElement('Filter')
    text_child = dom_tree.createTextNode('src')
    node_child.appendChild(text_child)
    child.appendChild(node_child)
    item_group.appendChild(child)

def append_compile_filter_node(dom_tree, item_group, f):
    child = dom_tree.createElement('ClCompile')
    child.setAttribute("Include", f)
    node_child = dom_tree.createElement('Filter')
    text_child = dom_tree.createTextNode('src')
    node_child.appendChild(text_child)
    child.appendChild(node_child)
    item_group.appendChild(child)

def save_xml(dom_tree, xml_file):
    f = open(xml_file, 'w')
    dom_tree.writexml(f, encoding='utf-8')
    f.close()

def remove_unused_include_node(dom_tree, filelist):
    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    for group in item_groups:
        includes = group.getElementsByTagName("ClInclude")
        for inc in includes:
            src_file = inc.getAttribute("Include")
            if not src_file.startswith("src\\"):
                continue
            if not src_file in filelist:
                #remove
                group.removeChild(inc)

def remove_unused_compile_node(dom_tree, filelist):
    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    for group in item_groups:
        includes = group.getElementsByTagName("ClCompile")
        for inc in includes:
            src_file = inc.getAttribute("Include")
            if not src_file.startswith("src\\"):
                continue
            if not src_file in filelist:
                # remove
                group.removeChild(inc)

def get_include_group_node(dom_tree):
    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    group = None
    for g in item_groups:
        names = g.getElementsByTagName('ClInclude')
        if names:
            group = g
            break

    if group is None:
        group = dom_tree.createElement('ItemGroup')
        project.appendChild(group)

    return group

def get_compile_group_node(dom_tree):
    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    group = None
    for g in item_groups:
        names = g.getElementsByTagName('ClCompile')
        if names:
            group = g
            break

    if group is None:
        group = dom_tree.createElement('ItemGroup')
        project.appendChild(group)

    return group

def append_filelist_to_project_filter_xml(xml_file, filelist):
    dom_tree = xml.dom.minidom.parse(xml_file)
    remove_unused_compile_node(dom_tree, filelist)
    remove_unused_include_node(dom_tree, filelist)

    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    change_flag = False
    for f in filelist:
        add_flag = False
        is_header = True if f.endswith('.h') else False
        for group in item_groups:
            names = group.getElementsByTagName('ClInclude') if is_header else group.getElementsByTagName('ClCompile')
            for name in names:
                src_file = name.getAttribute("Include")
                if f == src_file:
                    add_flag = True
                    break
            if add_flag:
                break

        if not add_flag:
            change_flag = True
            group = get_include_group_node(dom_tree) if is_header else get_compile_group_node(dom_tree)
            if is_header:
                append_include_filter_node(dom_tree, group, f)
            else:
                append_compile_filter_node(dom_tree, group, f)

    if change_flag:
        save_xml(dom_tree, xml_file)

def append_filelist_to_project_xml(xml_file, filelist):
    dom_tree = xml.dom.minidom.parse(xml_file)
    remove_unused_compile_node(dom_tree, filelist)
    remove_unused_include_node(dom_tree, filelist)

    project = dom_tree.documentElement
    item_groups = project.getElementsByTagName("ItemGroup")
    change_flag = False
    for f in filelist:
        add_flag = False
        is_header = True if f.endswith('.h') else False
        for group in item_groups:
            names = group.getElementsByTagName('ClInclude') if is_header else group.getElementsByTagName('ClCompile')
            for name in names:
                src_file = name.getAttribute("Include")
                if f == src_file:
                    add_flag = True
                    break
            if add_flag:
                break

        if not add_flag:
            change_flag = True
            group = get_include_group_node(dom_tree) if is_header else get_compile_group_node(dom_tree)
            if is_header:
                append_include_node(dom_tree, group, f)
            else:
                append_compile_node(dom_tree, group, f)

    if change_flag:
        save_xml(dom_tree, xml_file)


if __name__ == '__main__':
    if not os.path.exists('./src'):
        os.makedirs('./src/')
    files = os.listdir('./proto/')
    for f in files:
        if not f.endswith('.proto'):
            continue

        command = None
        if platform.system() == 'Windows':
            command = "..\\tool\\protoc.exe --proto_path=./proto --cpp_out=./src/ " + f
        else:
            command = "cd proto; protoc --cpp_out=../src/ " + f
        print (command)
        os.system(command)

    generator_build_file()

    source_files = []
    files = os.listdir('./src/')
    for f in files:
        source_files.append('src/' + f)
        append_filelist_to_project_filter_xml("protolib.vcxproj.filters", source_files)
        append_filelist_to_project_xml("protolib.vcxproj", source_files)


