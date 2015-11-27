-- initialize the server
server = ua.Server(16664)

-- entity objecttype
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.BaseObjectType
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "Entity")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Entity")
local entity_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)

-- interface objecttype
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.BaseObjectType
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "Interface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Interface")
local interface_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

-- connectedwith referencetype
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.NonHierarchicalReferences
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "ConnectedWith")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "ConnectedWith")
attr.symmetric = true
connectedwith_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                               reference_nodeid, browsename, attr)

-- administrationshell objecttype
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.BaseObjectType
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "AdministrationShell")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "AdministrationShell")
local as_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)
-- administrationshell identifier
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = as_id
local reference_nodeid = ua.nodeids.HasComponent
local browsename = ua.types.QualifiedName(1, "identifier")
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Identifier")
attr.value.value = ua.types.NodeId()
local as_identifier_id = server:addVariableNode(requested_nodeid, parent_nodeid,
                                                reference_nodeid, browsename, ua.nodeids.Null, attr)

-- administrates referencetype
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.NonHierarchicalReferences
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "Administrates")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Administrates")
attr.inversename = ua.types.LocalizedText("en_US", "AdministeredBy")
local administrates_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                                     reference_nodeid, browsename, attr)

-- usb interface
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = interface_id
local reference_nodeid = ua.nodeids.HasSubtype
local browsename = ua.types.QualifiedName(1, "USBInterface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBInterface")
local usb_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

local requested_nodeid = ua.nodeids.Null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.HasProperty
local browsename = ua.types.QualifiedName(1, "USBVersion")
local typeidentifier = ua.nodeids.Null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBVersion")
attr.value.value = ua.types.Int32()
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.Null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.HasProperty
local browsename = ua.types.QualifiedName(1, "USBConnectorType")
local typeidentifier = ua.nodeids.Null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBConnectorType")
attr.value.value = ua.types.Int32()
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

-- entities
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.Objects
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Entities")
local typeidentifier = ua.nodeids.FolderType
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Entities")
entities_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- office
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = entities_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Office")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Office")
office_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Computer")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Computer")
computer_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                   reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.Null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.HasProperty
local browsename = ua.types.QualifiedName(1, "Speed")
local typeidentifier = ua.nodeids.Null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Speed")
attr.value.value = ua.types.Int32()
id = server:addVariableNode(requested_nodeid, parent_nodeid,
                            reference_nodeid, browsename, typeidentifier, attr)

-- computer usb
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "ComputerUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "ComputerUSB")
local computerusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                            reference_nodeid, browsename, typeidentifier, attr)

-- mouse
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Mouse")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Mouse")
mouse_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.Null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.HasProperty
local browsename = ua.types.QualifiedName(1, "Resolution")
local typeidentifier = ua.nodeids.Null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Resolution")
id = server:addVariableNode(requested_nodeid, parent_nodeid,
                            reference_nodeid, browsename, typeidentifier, attr)

-- mouse usb
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "MouseUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "MouseUSB")
local mouseusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                         reference_nodeid, browsename, typeidentifier, attr)

-- mouse <--> computer usb
local source = mouseusb_id
local reftype = connectedwith_id
local target = computerusb_id
local isforward = true
server:addReference(source, reftype, target, isforward)

-- shells
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = ua.nodeids.Objects
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Administration Shells")
local typeidentifier = ua.nodeids.FolderType
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Administration Shells")
shells_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell
local requested_nodeid = ua.nodeids.Null
local parent_nodeid = shells_id
local reference_nodeid = ua.nodeids.Organizes
local browsename = ua.types.QualifiedName(1, "Computer Admin Shell")
local typeidentifier = as_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Computer Admin Shell")
cas_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                              reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell -> computer
local source = computer_id
local reftype = administrates_id
local target = cas_id
local isforward = false
server:addReference(source, reftype, target, isforward)

-- add method
function test(objectid, ...)
   print(objectid)
   return 1,2
end

browsename = ua.types.QualifiedName(1, "Test Method")
attr = ua.types.MethodAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Test Method")
attr.executable = true
server:addMethodNode(ua.nodeids.Null, ua.nodeids.Objects,
               ua.nodeids.HasComponent, browsename,
               attr, test, ua.Array(ua.types.Argument, 0), ua.Array(ua.types.Argument, 0))

server:start()
--server:stop()
