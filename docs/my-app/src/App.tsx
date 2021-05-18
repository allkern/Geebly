import * as React from "react"
import {
  ChakraProvider,
  Box,
  Text,
  Link,
  VStack,
  HStack,
  Center,
  SimpleGrid,
  Circle,
  Heading,
  Image,
  Button,
  Code,
  Grid,
  theme,
} from "@chakra-ui/react"
import { ColorModeSwitcher } from "./ColorModeSwitcher"
import { Logo } from "./Logo"

export const App = () => (
  <ChakraProvider theme={theme}>
      <VStack>
        <Center mt="50px" mb="50px">
          <Image width="70%" height="70%" src="https://user-images.githubusercontent.com/15825466/117024784-b1bc1300-acd0-11eb-9656-c4f6e1cc391c.png"/>
        </Center>
        <Box borderWidth="5px" bg="gray.200" borderRadius="xl">
          <Text fontSize="xl" ml="15px" mr="15px" mt="10px" mb="10px" color="gray.700">
            A Game Boy/Color Emulator for Windows, Linux and macOS
          </Text>
        </Box>
        <HStack mt="50px" spacing="50px">
          <Button>Windows</Button>
          <Button>Linux</Button>
          <Button>macOS</Button>
        </HStack>
      </VStack>
  </ChakraProvider>
)
