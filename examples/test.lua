local root = "/icore/eg/"
package.path = root .. "mod/?.lua;" .. root .. "mod/?/init.lua;" .. root .. "expand/?.lua;" .. root .. "business/?.lua;"
package.cpath = "../build/?.so"
local ua = require "ua"
local inspect = require "inspect"
-- initialize the server
server = ua.Server(16664)
print(inspect(ua.nodeIds))
print(inspect(ua.nodeIds.Null))
-- entity objecttype
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.BaseObjectType
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "Entity")
local attr = ua.types.ObjectTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Entity")
local entity_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)

-- interface objecttype
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.BaseObjectType
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "Interface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Interface")
local interface_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)

-- connectedwith referencetype
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.NonHierarchicalReferences
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "ConnectedWith")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "ConnectedWith")
attr.symmetric = true
connectedwith_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)

-- administrationshell objecttype
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.BaseObjectType
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "AdministrationShell")
local attr = ua.types.ObjectTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "AdministrationShell")
local as_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)
-- administrationshell identifier
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = as_id
local reference_nodeid = ua.nodeIds.HasComponent
local browsename = ua.types.QualifiedName(1, "identifier")
local attr = ua.types.VariableAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Identifier")
attr.value.value = ua.types.NodeId()
local as_identifier_id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, ua.nodeIds.Null, attr)

-- administrates referencetype
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.NonHierarchicalReferences
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "Administrates")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Administrates")
attr.inverseName = ua.types.LocalizedText("en_US", "AdministeredBy")
local administrates_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)

-- usb interface
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = interface_id
local reference_nodeid = ua.nodeIds.HasSubtype
local browsename = ua.types.QualifiedName(1, "USBInterface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "USBInterface")
local usb_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, attr)

local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeIds.HasProperty
local browsename = ua.types.QualifiedName(1, "USBVersion")
local typeidentifier = ua.nodeIds.Null
local attr = ua.types.VariableAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "USBVersion")
attr.value.value = ua.types.Int32()
usbversion_id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeIds.HasProperty
local browsename = ua.types.QualifiedName(1, "USBConnectorType")
local typeidentifier = ua.nodeIds.Null
local attr = ua.types.VariableAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "USBConnectorType")
attr.value.value = ua.types.Int32()
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- entities
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.Objects
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Entities")
local typeidentifier = ua.nodeIds.FolderType
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Entities")
entities_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- office
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = entities_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Office")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Office")
office_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- computer
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Computer")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Computer")
computer_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeIds.HasProperty
local browsename = ua.types.QualifiedName(1, "Speed")
local typeidentifier = ua.nodeIds.Null
local attr = ua.types.VariableAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Speed")
attr.value.value = ua.types.Int32()
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- computer usb
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "ComputerUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "ComputerUSB")
local computerusb_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- mouse
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Mouse")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Mouse")
mouse_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeIds.HasProperty
local browsename = ua.types.QualifiedName(1, "Resolution")
local typeidentifier = ua.nodeIds.Null
local attr = ua.types.VariableAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Resolution")
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- mouse usb
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "MouseUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "MouseUSB")
local mouseusb_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- mouse <--> computer usb
local source = mouseusb_id
local reftype = connectedwith_id
local target = computerusb_id
local isforward = true
server:addReference(source, reftype, target, isforward)

-- shells
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = ua.nodeIds.Objects
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Administration Shells")
local typeidentifier = ua.nodeIds.FolderType
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Administration Shells")
shells_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell
local requested_nodeid = ua.nodeIds.Null
local parent_nodeid = shells_id
local reference_nodeid = ua.nodeIds.Organizes
local browsename = ua.types.QualifiedName(1, "Computer Admin Shell")
local typeidentifier = as_id
local attr = ua.types.ObjectAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Computer Admin Shell")
cas_id = server:addObjectNode(requested_nodeid, parent_nodeid, reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell -> computer
local source = computer_id
local reftype = administrates_id
local target = cas_id
local isforward = false
server:addReference(source, reftype, target, isforward)

-- add method
function test(objectid, ...)
	print(objectid)
	return 1, 2
end

browsename = ua.types.QualifiedName(1, "Test Method")
attr = ua.types.MethodAttributes()
attr.displayName = ua.types.LocalizedText("en_US", "Test Method")
attr.executable = true
server:addMethodNode(ua.nodeIds.Null, ua.nodeIds.Objects, ua.nodeIds.HasComponent, browsename, attr, test, ua.Array(ua.types.Argument, 0), ua.Array(ua.types.Argument, 0))

res = server:write(usbversion_id, ua.attributeIds.Value, ua.types.Int32(5))

server:start()
while true do
	server:iterate()
end
--server:stop()
