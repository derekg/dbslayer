t = {}
schema =  Json.Decode( schema_json )
map = {}

for k,v in pairs(schema.SCHEMA) do 
    tn = string.format("%s", k)
    map[tn]={ table=tn,
    	      key="id",
	      class=td,
	      members={}
	      }
    if type(v) == "table" then      
        for k1, v1 in pairs(v) do
  	   mn = string.format("%s", k1)
	   map[tn]["members"][mn] = string.gsub(v1, "MYSQL_TYPE_", "", 1)
	end
    end
end
--io.write(string.format("%s", Json.Encode(map)) .. "\n")
--io.flush(io.stdout)
map.Country.key="Code"
map.Building.members.city = { class="City", relation="many_to_one", key  = "city_id", table="Building" }

map.City.members.buildings = { class="Building",
                	      relation="one_to_many",
       		       	      key="city_id",
		       	      table="Building" }

map.Building.members.apartments = {  class =  "Apartment",
   		                    relation="one_to_mane",
               		      	   key="building_id",
				   table="Apartment" }

map.Apartment.members.building = {  class =  "Building",
   		                    relation="many_to_one",
               		      	   key="building_id",
				   table="Apartment" }

map.Apartment.members.building = {  class =  "Building",
   		                    relation="many_to_one",
               		      	   key="building_id",
				   table="Apartment" }

map.Apartment.members.tenants = { class = "Apartment",
     	                          relation="many_to_one",
               		      	  key="apartment_id",
				  table="Tenant" }

map.Tenant.members.apartment = {  class =  "Apartment",
     	                         relation="many_to_one",
               		      	  key="apartment_id",
				  table="Tenant" }

map.Tenant.members.memberships = {  class="Membership",
			            relation="one_to_many", 
				    key="tenant_id",
				    table="Membership"  }

map.Tenant.members.community_associations = { class="ComminunityAssociation", 
					      relation="many_to_many",
					      table="Membership",
					      key="community_association_id",
					      self_key="tenant_id" }

map.Membership.members.tenant = {  class="Membership",
			            relation="one_to_many", 
				    key="tenant_id",
				    table="Membership"  }

map.Membership.members.community_assocation = {  class="Membership",
			            relation="one_to_many", 
				    key="community_association_id",
				    table="Membership"  }

map.CommunityAssociation.members.memberships = {  class="Membership",
			            relation="one_to_many", 
				    key="community_association_id",
				    table="Membership"  }

map.CommunityAssociation.members.memberships = {  class="Membership",
			            relation="one_to_many", 
				    key="community_association_id",
				    table="Membership"  }

map.CommunityAssociation.members.tenants = {  class="Tenant",
					      relation="many_to_many",
					      table="Membership",
					      key="tenant_id", 
					      self_key="community_association_id"}

-- undefine TenantLanguge, as if we only want to allow access via the join

map.TenantLanguage = nil

map.Tenant.members.languages = { class="CountryLanguage",
			         relation="many_to_many",
				 table="TenantLanguage",
				 key="CountryCode", 
				 self_key="tenant_id" }

map.CountryLanguage.members.tenants = { class="CountryLanguage",
        			        relation="many_to_many",
    	 			        table="TenantLanguage",
  				        key="tenant_id",
   				        self_key="CountryCode" }



				 
t.RESULT = {}
t.RESULT.MAP = map			 
t.SERVER = server
return Json.Encode(t)
