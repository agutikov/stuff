#!/bin/env python3


import sys
import os
import binascii
import struct
from pprint import pprint
from subprocess import Popen, PIPE


page_size = os.sysconf("SC_PAGE_SIZE")
pagemap_entry_size = 8


def get_vma_list(pid):
    vmas = {}
    with open(f'/proc/{pid}/maps', 'r') as f:
        for line in f.readlines():
            if len(line) > 0:
                addrs = line.split()[0]
                addrs = addrs.split('-')
                vmas[int(addrs[0], 16)] = (int(addrs[1], 16), line.strip())
    return vmas



def is_present(entry):
  return ((entry & (1 << 63)) != 0)

def get_pfn(entry):
  return entry & 0x7FFFFFFFFFFFFF

def get_vma_physical_pages(pid, start, end):
    pages = {}
    start_offset = int(start / page_size) * pagemap_entry_size
    pagemap_entry_count = int((end - start) / page_size)
    with open(f'/proc/{pid}/pagemap', 'rb') as f:
        f.seek(start_offset, 0)
        data = f.read(pagemap_entry_count * pagemap_entry_size)
    if len(data) > 0:
        for i in range(pagemap_entry_count):
            pagemap_entry = struct.unpack('Q', data[i * pagemap_entry_size : (i + 1) * pagemap_entry_size])[0]
            if is_present(pagemap_entry):
                pages[get_pfn(pagemap_entry) * page_size] = (get_pfn(pagemap_entry), start + i * page_size)
    return pages


def get_dmidecode():
    d = {}
    process = Popen(["dmidecode"], stdout=PIPE)
    (output, err) = process.communicate()
    exit_code = process.wait()
    if exit_code == 0:
        for block in output.decode('utf-8').split('\n\n'):
            if len(block) == 0:
                continue
            if block[0] == '#':
                continue
            lines = block.split('\n')
            x = lines[0].split(',')
            item = {
                "handle": int(x[0].split()[1], 16),
                "dma_type_id": int(x[1].split()[2]),
                "dma_type": lines[1]
            }
            for line in lines[2:]:
                if ':' in line:
                    a = line.split(':')
                    key = a[0].strip()
                    value = ":".join(a[1:])
                    item[key] = value.strip()
                else:
                    item[key] += " " + line.strip()
            d[item["handle"]] = item 
    return d

def get_memory_devices(dmidecode):
    mem_devices = {}
    for handle, mem_addr in dmidecode.items():
        if mem_addr['dma_type_id'] == 20:
            mem_hw = dmidecode[int(mem_addr['Physical Device Handle'], 16)]
            mem_devices[(int(mem_addr["Starting Address"], 16), int(mem_addr["Ending Address"], 16))] = (mem_addr, mem_hw)
    return mem_devices

def find_mem_item(mem_devs, addr):
    for addrs, item in mem_devs.items():
        if addr >= addrs[0] and addr <= addrs[1]:
            return addrs, item
    return None, None

def main():
    if len(sys.argv) == 2:
        pid = sys.argv[1]
    else:
        pid = os.getpid()

    dmidecode = get_dmidecode()
    #pprint(dmidecode)
    mem_devs = get_memory_devices(dmidecode)
    #pprint(mem_devs)

    vmas = get_vma_list(pid)
    proc_stat = {}
    for start, vma in vmas.items():
        #vmas[start] = (vma[0], vma[1], get_vma_physical_pages(pid, start, vma[0]))
        pages = get_vma_physical_pages(pid, start, vma[0])
        for page_start_addr, page in pages.items():
            addrs, item = find_mem_item(mem_devs, page_start_addr)
            if item is not None:
                pages[page_start_addr] += (item[1]['Locator'], item[1]['Bank Locator'], )
        vma += (pages, )

        print(vma[1])
        vma_stat = {}
        for page_start_addr, page in pages.items():
            print("0x%x 0x%x" % (page[1], page_start_addr), end='')
            if len(page) >= 3:
                locator = page[2]
                bank = page[3]
                print(' %s %s' % (locator, bank))
                if bank not in vma_stat:
                    vma_stat[bank] = {}
                vma_stat[bank][locator] = vma_stat[bank].get(locator, 0) + 1
            else:
                print()
    
        if len(page) >= 3:
            print("= VMA STATS " + "=" * 40)
            for bank, bank_item in vma_stat.items():
                for locator, locator_count in bank_item.items():
                    print(bank, locator, locator_count)
                    if bank not in proc_stat:
                        proc_stat[bank] = {}
                    proc_stat[bank][locator] = proc_stat[bank].get(locator, 0) + locator_count
        print()
    
    if len(page) >= 3:
        print("== TOTAL STATS " + "=" * 80)
        for bank, bank_item in proc_stat.items():
            for locator, locator_count in bank_item.items():
                print(bank, locator, locator_count)




if __name__ == "__main__":
    main()
