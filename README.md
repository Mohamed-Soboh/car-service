# car-service
The car service app is a console-based application running on Linux. It will help manage appointments, 
services, and customer information for a car service center.

1. Resource Class (resource.h):
Description: Represents a resource within the car service application. This could be physical resources like tools or equipment.
Properties:
Define properties specific to a resource (e.g., resource ID, status).
Methods:
start_resource(Resource* resource): Initiates the usage of the resource.
give_resource(Resource* resource): Marks the resource as available for others.
free_resources(Resource* resources): Releases memory occupied by resources.
2. Service Class (service.h):
Description: Represents a service offered by the car service center.
Properties:
Define properties specific to a service (e.g., service ID, type).
Methods:
get_services(): Retrieves a list of available services.
get_service_index(Service* services, const char* serviceType): Finds the index of a service based on its type.
isAvailable(Service* services, int index): Checks the availability status of a service.
free_services(Service* services): Releases memory occupied by services.
3. Request Type (request.h):
Description: Represents a request made by a customer for a specific service.
Properties:
Define properties specific to a request (e.g., request ID, status).
Methods:
car_request(Request* request): Initiates a car service request.
free_requests(Request* requests): Releases memory occupied by requests.
4. Time Functions (time.h):
Methods:
start_time(): Initializes the system time or tracks the start time of a process.
start_garage(): Signals the start of operations in the garage.
5. Main Function (main.h):
Description: The primary entry point for the car service application.
Methods:
main(): Contains the main logic and user interface for the application.
free_all(Resource* resources, Service* services, Request* requests): 
